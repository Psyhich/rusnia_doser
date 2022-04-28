#ifndef TCP_GUN_H
#define TCP_GUN_H

#include "curl_wrapper.h"
#include "gun.h"

namespace Attackers
{

class TCPGun : public IGun
{
public:
	std::optional<Target> Aim(const CURI &uriToAttack) noexcept override;
	void FireTillDead(const Target &targetToKill) noexcept override;

	bool FireWithoutProxy(const Target &targetToKill) noexcept;
	void FireWithProxy(const Target &targetToKill) noexcept;
private:

	bool SetValidProxy() noexcept;
	std::optional<Proxy> ChoseProxy(const std::vector<Proxy> &proxies) noexcept;
private:
	std::optional<Proxy> m_currentProxy;
};


} // Attackers 


#endif // TCP_GUN_H
