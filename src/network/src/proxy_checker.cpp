#include "proxy_checker.h"

#include "config.h"
#include "wrappers/http_wrapper.h"
#include "net_utils.h"

namespace NetUtil
{
	ProxyList DefaultHostProxyChecker::CheckProxies(const ProxyList &proxiesToCheck,
		const TaskController &ownerTask)
	{
		ProxyList workingProxies;

		SPDLOG_INFO("Probing proxies with default host: {}", ProxyConfig::PROXY_PROBING);

		HTTPWrapper httpPinger;
		Headers headers{HTTPWrapper::BASE_HEADERS};
		httpPinger.SetTarget(ProxyConfig::PROXY_PROBING);
		for(const auto &proxy : proxiesToCheck)
		{
			httpPinger.SetProxy(proxy.first, proxy.second);
			UpdateHeaders(headers, proxy.first);
			httpPinger.SetHeaders(headers);

			std::size_t proxyProbingTries = 0;
			while(proxyProbingTries < ProxyConfig::PROXY_PROBING_TRIES)
			{
				if(const auto respCode{httpPinger.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS)})
				{
					if(*respCode >= 200 && *respCode < 300)
					{
						workingProxies.push_back(proxy);
						break;
					}
					SPDLOG_INFO("Proxy: {}@{} failed with code: {}", proxy.first, proxy.second, *respCode);
				}
				else
				{
					SPDLOG_INFO("Proxy: {}@{} failed withot any code", proxy.first, proxy.second);
				}

				if(ownerTask.ShouldStop())
				{
					return workingProxies;
				}
				++proxyProbingTries;
			}
		}

		return workingProxies;
	}
}
