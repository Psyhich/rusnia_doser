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
#include "globals.h"

void signalHanlder(int signum) 
{
	std::cerr << "Stoping" << std::endl;
	g_shouldStop.store(true);
}

int main(int argc, char **argv)
{
	signal(SIGHUP, signalHanlder);
	signal(SIGTERM, signalHanlder);
	signal(SIGABRT, signalHanlder);
	signal(SIGINT, signalHanlder);

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

	std::vector<std::thread> pool;
	for(size_t i = 0; i < maxThreads; i++)
	{
		pool.push_back(std::thread(HTTPFire, apis, std::ref(g_shouldStop)));
	}

	for(auto &thread : pool)
	{
		if(thread.joinable())
		{
			thread.join();
		}
	}
	std::cout << "Succesfully stoped" << std::endl;
}
