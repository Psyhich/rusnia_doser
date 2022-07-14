#ifndef SOLIDER_H
#define SOLIDER_H

#include <thread>

#include "globals.h"
#include "multithread.h"
#include "target.hpp"

class Solider
{
public:
	Solider(Attackers::PTarget &&target) : 
		m_target{std::move(target)}
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

		return *this;
	}

	inline void StartExecution()
	{
		m_task.StartExecution(std::ref(ExecuteOrders), std::cref(m_task), std::ref(*m_target));
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
	static void ExecuteOrders(const TaskController &task, Attackers::Target &target);
	
private:
	TaskController m_task;

	Attackers::PTarget m_target;
};

#endif // SOLIDER_H
