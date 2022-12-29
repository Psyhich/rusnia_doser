#ifndef API_INTERFACE_H
#define API_INTERFACE_H

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <mutex>

#include "nlohmann/json.hpp"

#include "curl_wrapper.h"
#include "target.hpp"
#include "multithread.h"

class IProxyGetter
{
public:
	virtual std::optional<std::vector<Proxy>> GetProxies() = 0;
};
using SPProxyGetter = std::shared_ptr<IProxyGetter>;

class EmptyProxyGetter : public IProxyGetter
{
	std::optional<std::vector<Proxy>> GetProxies() override
	{
		return std::nullopt;
	}
};


#endif // API_INTERFACE_H
