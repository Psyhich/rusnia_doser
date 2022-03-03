#ifndef PROXY_MANAGER_H
#define PROXY_MANAGER_H

#include <map>
#include <string>
#include <mutex>

using Proxy = std::pair<std::string, std::string>;
// manages valid and non-valid proxies by giving 
// them starting rating and decreasing it each time 
// the manager cannot access it
class ProxyManager
{
	using ProxyRating = std::pair<Proxy, int>;
public:
	ProxyManager(const std::string &resouces);
	
	Proxy GetProxy();
	
private:
	ProxyRating FindProxy(const Proxy &proxyToFind);
	bool LowerRating(const Proxy &proxyToLower);

private:
	inline static constexpr const short STARTING_RATING = 4;

	std::string m_resourceServer;

	std::mutex m_ratingMutex;
	std::map<Proxy, short> m_proxyRating;

};

#endif // PROXY_MANAGER_H
