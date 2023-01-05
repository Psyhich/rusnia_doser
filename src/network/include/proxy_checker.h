#ifndef PROXY_CHECKER_H
#define PROXY_CHECKER_H

#include <vector>

#include "api_interface.h"
#include "multithread.h"

namespace NetUtil
{

using ProxyList = std::vector<Proxy>;

class IProxyChecker
{
public:
	virtual ProxyList 
		CheckProxies(const ProxyList &proxiesToCheck, const TaskController &ownerTask) = 0;
	virtual ~IProxyChecker() = default;
private:
};

class DefaultHostProxyChecker : public IProxyChecker
{
public:
	ProxyList CheckProxies(const ProxyList &proxiesToCheck, const TaskController &ownerTask) override;
	~DefaultHostProxyChecker() override = default;
};

}

#endif // PROXY_CHECKER_H
