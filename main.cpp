#include <signal.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include "nlohmann/json.hpp"

#include "config.h"
#include "solider.h"
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
	signal(SIGTERM, signalHanlder);

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
		Solider solider;
		pool.emplace_back(solider);
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
