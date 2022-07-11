#ifndef TCP_GUN_H
#define TCP_GUN_H

#include "tcp_wrapper.h"
#include "curl_wrapper.h"

#include "gun.hpp"

namespace Attackers
{

class TCPGun : public IGun
{
public:
	TCPGun(const TaskController &task) : IGun(task)
	{ }

	std::optional<CURI> Aim(const CURI &uriToAttack) noexcept override;
	void FireTillDead(const CURI &targetToKill) noexcept override;

	bool FireWithoutProxy(const CURI &targetToKill) noexcept;
	void FireWithProxy(const CURI &targetToKill) noexcept;
private:
	bool ShootTarget(const CURI &targetToKill);

private:
	std::optional<Proxy> m_currentProxy;
	TCPWrapper m_attacker;
};


} // Attackers 


#endif // TCP_GUN_H
