#ifndef HTTP_GUN_H
#define HTTP_GUN_H

#include <deque>

#include "api_interface.h"
#include "curl_wrapper.h"
#include "multithread.h"
#include "target.hpp"
#include "gun.hpp"
#include "utils.h"

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
	std::size_t FireTillDead(const CURI &targetToKill) noexcept override;

private:
	TargetStatus FireGun();

	bool AttackWithNoProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept;
	void AttackWithProxy(const CURI &targetToKill, std::size_t &hitsCount) noexcept;

	bool LoadProxies();

	inline void SetupNonProxyHeaders()
	{
		m_headers["User-Agent"] = ChoseUseragent();
		m_wrapper.SetHeaders(m_headers);
	}

	inline void SetupProxyHeaders(const std::string &proxyIP)
	{
		UpdateHeaders(m_headers, proxyIP);
		m_wrapper.SetHeaders(m_headers);
	}
private:
	std::deque<Proxy> m_availableProxies;

	Headers m_headers{CURLLoader::BASE_HEADERS};
	CURLLoader m_wrapper;
	SPProxyGetter m_proxyGetter;
};

} // Attackers 

#endif // HTTP_GUN_H
