#ifndef API_INTERFACE_H
#define API_INTERFACE_H

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <mutex>
#include <map>

#include "multithread.h"
#include "http_structs.hpp"

class IProxyGetter
{
public:
	virtual std::optional<std::vector<HTTP::Proxy>> GetProxies() = 0;
};
using SPProxyGetter = std::shared_ptr<IProxyGetter>;

class EmptyProxyGetter : public IProxyGetter
{
	std::optional<std::vector<HTTP::Proxy>> GetProxies() override
	{
		return std::nullopt;
	}
};


#endif // API_INTERFACE_H
