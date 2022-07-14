#ifndef HTTP_GUN_H
#define HTTP_GUN_H

#include "curl_wrapper.h"
#include "multithread.h"
#include "target.hpp"
#include "gun.hpp"

namespace Attackers
{

class HTTPGun : public IGun
{
public:
	HTTPGun(const TaskController &task) : IGun(task)
	{}
	std::size_t FireTillDead(const CURI &targetToKill) noexcept override;

private:
	bool AttackWithNoProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept;
	void AttackWithProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept;

	bool SetValidProxy() noexcept;
	std::optional<Proxy> ChoseProxy(const std::vector<Proxy> &proxies) noexcept;

private:
	std::optional<Proxy> m_currentProxy;
};

} // Attackers 

#endif // HTTP_GUN_H
