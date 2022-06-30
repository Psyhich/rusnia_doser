#include <csignal>

#include <optional>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

#include "args-parser/all.hpp"

#include "logging.h"
#include "api_interface.h"
#include "config.h"
#include "solider.h"
#include "multithread.h"

static TaskController g_mainTask;

using namespace Args;

void signalHanlder(int signum) 
{
	SPDLOG_WARN("Stopping, please be patient");
	g_mainTask.StopExecution();
}

bool SetupArgs(CmdLine &cmd)
{
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
		return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	std::signal(SIGTERM, signalHanlder);
	std::signal(SIGINT, signalHanlder);
	std::signal(SIGHUP, signalHanlder);

	CmdLine cmd{argc, argv};
	if(!SetupArgs(cmd))
	{
		return -1;
	}

	if(!SetupLogging())
	{
		return -1;
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
			SPDLOG_CRITICAL("Failed to parse port number");
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
			SPDLOG_CRITICAL("TCP attack should have port!");
			return -1;
		}
	}
	else if(methodString == "udp")
	{
		SPDLOG_ERROR("Not implemented yet");
		return -1;
	}
	else
	{
		SPDLOG_ERROR("Failed to parse: {} to known methods!", methodString);
		return -1;
	}

	size_t squadSize = 
		std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();

	if(cmd.isDefined("--threads"))
	{
		const std::string threadsString = cmd.value("--threads");
		try
		{
			squadSize = std::stoull(threadsString);
		}
		catch(...)
		{
			SPDLOG_ERROR("Failed to parse {} to number", threadsString);
			return -1;
		}
	}

	SPDLOG_INFO("Beggining attack using {} method", methodString);
	SPDLOG_INFO("Dispatching {} soliders", squadSize);

	std::vector<Solider> squad;
	squad.reserve(squadSize);

	for(size_t i = 0; i < squadSize; i++)
	{
		squad.emplace_back(attackMethod, target, port.value_or(0))
			.StartExecution();
	}

	while(!g_mainTask.ShouldStop()) { }

	for(auto &solider : squad)
	{
		solider.StopExecution();
	}

	SPDLOG_INFO("Attack finished");
	spdlog::drop_all();
}
