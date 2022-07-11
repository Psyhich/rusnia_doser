#ifndef SOLIDER_H
#define SOLIDER_H

#include <thread>

#include "globals.h"
#include "api_interface.h"
#include "http_gun.h"
#include "multithread.h"
#include "target.hpp"
#include "tcp_gun.h"

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
		m_task.StartExecution();

		std::thread toBeSwapped{&ExecuteOrders, std::cref(m_task), std::ref(*m_target)};
		m_runningOrder.swap(toBeSwapped);
	}

	void StopExecution()
	{
		m_task.StopExecution();
		if(m_runningOrder.joinable())
		{
			m_runningOrder.join();
		}
	}

	bool ShouldStop() const
	{
		return m_task.ShouldStop();
	}

private:
	static void ExecuteOrders(const TaskController &task, Attackers::Target &target);
	
private:
	TaskController m_task;
	std::thread m_runningOrder;

	Attackers::PTarget m_target;
};

#endif // SOLIDER_H
