#ifndef HTTP_GUN_H
#define HTTP_GUN_H

#include "gun.h"
#include "curl_wrapper.h"

namespace Attackers
{

class HTTPGun : public IGun
{
public:
	std::optional<Target> Aim(const CURI &uriToAttack) noexcept override;
	void FireTillDead(const Target &targetToKill) noexcept override;
private:
	bool AttackWithNoProxy(const Target &targetToKill) noexcept;
	void AttackWithProxy(const Target &targetToKill) noexcept;

	bool SetValidProxy() noexcept;
	std::optional<Proxy> ChoseProxy(const std::vector<Proxy> &proxies) noexcept;
private:
	std::optional<Proxy> m_currentProxy;
};

} // Attackers 

#endif // HTTP_GUN_H
