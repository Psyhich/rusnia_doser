#ifndef TCP_GUN_H
#define TCP_GUN_H

#include "api_interface.h"
#include "tcp_wrapper.h"
#include "curl_wrapper.h"

#include "gun.hpp"

namespace Attackers
{

class TCPGun : public IGun
{
public:
	TCPGun(const TaskController &task, SPProxyGetter proxyGetter) :
		IGun(task),
		m_proxyGetter{proxyGetter}
	{ }

	std::size_t FireTillDead(const URI &targetToKill) noexcept override;

	bool FireWithoutProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept;
	void FireWithProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept;
private:
	bool ShootTarget(const URI &targetToKill, std::size_t &hitsCount);

	bool LoadProxies();
private:
	TCPWrapper m_attacker;
	SPProxyGetter m_proxyGetter;
};


} // Attackers 


#endif // TCP_GUN_H
