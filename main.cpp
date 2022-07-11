#include <csignal>

#include <optional>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

#include <args-parser/all.hpp>

#include "logging.h"
#include "setup.h"
#include "solider.h"
#include "multithread.h"
#include "dynamic_target.h"
#include "static_target.hpp"

static TaskController g_mainTask;

Attackers::PTarget ProduceTarget(const Attackers::Tactic &tactic, const Args::CmdLine &)
{
	if(tactic.isAutoAim)
	{
		return std::make_unique<Attackers::DynamicTarget>();
	}
	else
	{
		return std::make_unique<Attackers::StaticTarget>(tactic.coordintates, tactic.method);
	}
}

void signalHanlder(int) 
{
	g_mainTask.StopExecution();
}

int main(int argc, char **argv)
{
	std::signal(SIGTERM, signalHanlder);
	std::signal(SIGINT, signalHanlder);
	std::signal(SIGHUP, signalHanlder);

	Args::CmdLine cmd{argc, argv};
	if(!Setup::SetupArgs(cmd))
	{
		return -1;
	}

	if(!Setup::SetupLogging())
	{
		return -1;
	}

	auto tactic = Setup::GetTactic(cmd);
	if(!tactic)
	{
		return -1;
	}

	g_mainTask.StartExecution();
	Informator::ContactSources(g_mainTask);

	// Basic setup is done checking tactic and starting attack

	if(tactic->isAutoAim)
	{
		SPDLOG_INFO("Beggining auto attack");
	}
	else
	{
		SPDLOG_INFO("Beggining attack using {} method", tactic->method);
	}
	SPDLOG_INFO("Dispatching {} soliders", tactic->squadSize);

	const Attackers::PTarget target{ProduceTarget(*tactic, cmd)};

	std::vector<Solider> squad;
	squad.reserve(tactic->squadSize);

	for(size_t i = 0; i < tactic->squadSize; i++)
	{
		squad.emplace_back(target->Clone())
			.StartExecution();
	}

	while(!g_mainTask.ShouldStop())
	{ }

	SPDLOG_WARN("Stopping, please be patient");

	for(auto &solider : squad)
	{
		solider.StopExecution();
	}

	SPDLOG_INFO("Attack finished");
	spdlog::drop_all();
}
