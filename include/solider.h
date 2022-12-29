#ifndef SOLIDER_H
#define SOLIDER_H

#include <memory>
#include <thread>

#include "api_interface.h"
#include "gun.hpp"
#include "multithread.h"
#include "target.hpp"

class Solider
{
public:
	Solider(Attackers::PTarget &&target, SPProxyGetter proxyGetter) : 
		m_target{std::move(target)},
		m_proxyGetter{proxyGetter}
	{}

	Solider(const Solider &copy) : 
		m_target{copy.m_target->Clone()}
	{ }

	Solider &operator=(const Solider &copy) 
	{
		if(&copy == this)
		{
			return *this;
		}

		m_target = copy.m_target->Clone();
		m_proxyGetter = copy.m_proxyGetter;

		return *this;
	}

	Solider(Solider &&move) : 
		m_target{std::move(move.m_target)}
	{ }

	Solider &operator=(Solider &&move) 
	{
		if(&move == this)
		{
			return *this;
		}

		m_target = std::move(move.m_target);
		m_proxyGetter = std::move(move.m_proxyGetter );

		return *this;
	}

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
		const TaskController &owningTask, SPProxyGetter proxyGetter);
	
private:
	TaskController m_task;

	Attackers::PTarget m_target;
	SPProxyGetter m_proxyGetter;
};

#endif // SOLIDER_H
