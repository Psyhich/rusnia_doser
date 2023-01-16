#ifndef TCP_GUN_H
#define TCP_GUN_H

#include "api_interface.h"
#include "wrappers/tcp_wrapper.h"
#include "resolvers.h"
#include "gun.hpp"

namespace Attackers
{

class TCPGun : public IGun
{
public:
	TCPGun(const TaskController &task, NetUtil::PAddressResolver resolver) :
		IGun(task),
		m_attacker{resolver}
	{ }

	std::size_t FireTillDead(const URI &targetToKill) noexcept override;

private:
	bool ShootTarget(const URI &targetToKill, std::size_t &hitsCount);

private:
	TCPWrapper m_attacker;
};


} // Attackers 


#endif // TCP_GUN_H
