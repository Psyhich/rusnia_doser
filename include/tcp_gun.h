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

	std::size_t FireTillDead(const CURI &targetToKill) noexcept override;

	bool FireWithoutProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept;
	void FireWithProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept;
private:
	bool ShootTarget(const CURI &targetToKill, std::size_t &hitsCount);

private:
	std::optional<Proxy> m_currentProxy;
	TCPWrapper m_attacker;
};


} // Attackers 


#endif // TCP_GUN_H
