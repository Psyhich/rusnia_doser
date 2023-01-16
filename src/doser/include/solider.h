#ifndef SOLIDER_H
#define SOLIDER_H

#include <memory>
#include <thread>

#include "api_interface.h"
#include "gun.hpp"
#include "multithread.h"
#include "resolvers.h"
#include "target.hpp"

class Solider
{
public:
	Solider(Attackers::PTarget target, SPProxyGetter proxyGetter) : 
		m_target{target},
		m_proxyGetter{proxyGetter}
	{}

	Solider(const Solider &copyFrom) = delete;
	Solider &operator=(const Solider &copyFrom) = delete;

	Solider(Solider &&moveFrom) = default;
	Solider &operator=(Solider &&moveFrom) = default;

	inline void StartExecution()
	{
		m_task.StartExecution(std::ref(ExecuteOrders), std::cref(m_task), std::ref(*m_target), m_proxyGetter);
	}

	inline void SendStop()
	{
		m_task.SendStop();
	}

	inline void WaitTillEnd()
	{
		m_task.UtilizeTask();
	}

	inline bool ShouldStop() const
	{
		return m_task.ShouldStop();
	}

private:
	static void ExecuteOrders(const TaskController &task,
		Attackers::Target &target, SPProxyGetter proxyGetter);

	static Attackers::PGun GunFactory(Attackers::AttackMethod attackMethod,
		const TaskController &owningTask, SPProxyGetter proxyGetter,
		NetUtil::PAddressResolver tcpAddressResolver,
		NetUtil::PAddressResolver udpAdressResolver);
	
private:
	TaskController m_task;

	Attackers::PTarget m_target;
	SPProxyGetter m_proxyGetter;
};

#endif // SOLIDER_H
