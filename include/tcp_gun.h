#ifndef TCP_GUN_H
#define TCP_GUN_H

#include "tcp_wrapper.h"
#include "curl_wrapper.h"

#include "gun.h"

namespace Attackers
{

class TCPGun : public IGun
{
public:
	TCPGun(const TaskController &task) : IGun(task)
	{ }

	std::optional<Target> Aim(const CURI &uriToAttack) noexcept override;
	void FireTillDead(const Target &targetToKill) noexcept override;

	bool FireWithoutProxy(const Target &targetToKill) noexcept;
	void FireWithProxy(const Target &targetToKill) noexcept;
private:
	bool ShootTarget(const Target &targetToKill);

private:
	std::optional<Proxy> m_currentProxy;
	TCPWrapper m_attacker;
};


} // Attackers 


#endif // TCP_GUN_H
