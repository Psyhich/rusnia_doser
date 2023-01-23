#include "gun_holder.h"
#include <stdexcept>

namespace Attackers
{

bool Gun::EmplaceGun(Attackers::AttackMethod attackMethod,
	const TaskController &owningTask, Wrappers::HTTP::SPProxyGetter proxyGetter,
	NetUtil::PAddressResolver tcpAddressResolver,
	NetUtil::PAddressResolver udpAdressResolver)
{
	switch(attackMethod)
	{
		case Attackers::AttackMethod::HTTPAttack:
		{
			emplace<Attackers::HTTPGun>(std::cref(owningTask), proxyGetter);
			return true;
		}
		case Attackers::AttackMethod::TCPAttack:
		{
			emplace<Attackers::TCPGun>(std::cref(owningTask), tcpAddressResolver);
			return true;
		}
		case Attackers::AttackMethod::UDPAttack:
		{
			emplace<Attackers::UDPGun>(std::cref(owningTask), udpAdressResolver);
			return true;
		}
		default:
		{
			SPDLOG_ERROR("Got unsupported attacker");
			return false;
		}
	}
}

}
