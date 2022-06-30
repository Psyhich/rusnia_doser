#ifndef SOLIDER_H
#define SOLIDER_H

#include <thread>

#include "globals.h"
#include "api_interface.h"
#include "http_gun.h"
#include "multithread.h"
#include "tcp_gun.h"

class Solider
{
public:
	Solider(Informator::AttackMethod method, const std::string &target, int port=0) : 
		m_method{method},
		m_target{target},
		m_port{port}
	{}

	Solider(const Solider &copy) : 
		m_method{copy.m_method},
		m_target{copy.m_target},
		m_port{copy.m_port}
	{ }
	Solider &operator=(const Solider &copy) 
	{
		if(&copy == this)
		{
			return *this;
		}

		m_method = copy.m_method;
		m_target = copy.m_target;
		m_port = copy.m_port;

		return *this;
	}

	Solider(const Solider &&move) : 
		m_method{move.m_method},
		m_target{std::move(move.m_target)},
		m_port{move.m_port}
	{ }
	Solider &operator=(const Solider &&move) 
	{
		if(&move == this)
		{
			return *this;
		}

		m_method = move.m_method;
		m_target = std::move(move.m_target);
		m_port = move.m_port;

		return *this;
	}

	inline void StartExecution()
	{
		m_task.StartExecution();
		std::thread toBeSwapped{&ExecuteOrders, std::cref(m_task), 
			m_method, Attackers::Target{m_target, m_port}};
		m_runningOrders.swap(toBeSwapped);
	}

	void StopExecution()
	{
		m_task.StopExecution();
		if(m_runningOrders.joinable())
		{
			m_runningOrders.join();
		}
	}

	bool ShouldStop() const
	{
		return m_task.ShouldStop();
	}

private:
	static void ExecuteOrders(const TaskController &task, 
		Informator::AttackMethod method, Attackers::Target target);
	
private:
	TaskController m_task;
	std::thread m_runningOrders;

	Informator::AttackMethod m_method;
	std::string m_target;
	int m_port;
};

#endif // SOLIDER_H
