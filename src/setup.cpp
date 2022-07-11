#include <thread>

#include "setup.h"

bool Setup::SetupArgs(Args::CmdLine &cmd)
{
	try
	{
		cmd.addArgWithNameOnly("target", true, true, "URI of target")
			.addArgWithNameOnly("threads", true, false, "Count of threads to use");
		cmd.parse();
	}
	catch(const Args::BaseException &x)
	{
		std::cerr << x.what() << ": " << x.desc() << std::endl;
		return false;
	}
	return true;
}

std::optional<Attackers::Tactic> Setup::GetTactic(const Args::CmdLine &cmd)
{
	Attackers::Tactic attackTactic;

	const CURI target = CURI{cmd.value("--target")};
	if(target.GetFullURI() == "auto")
	{
		attackTactic.isAutoAim = true;
	}

	std::optional<int> port{target.GetPort()};

	const auto method = target.GetProtocol();
	if(!attackTactic.isAutoAim)
	{
		if(method)
		{
			if(*method == "http" || *method == "https")
			{
				attackTactic.method = Attackers::AttackMethod::HTTPAttack;
			}
			else if(*method == "tcp")
			{
				attackTactic.method = Attackers::AttackMethod::TCPAttack;
				if(!port)
				{
					SPDLOG_CRITICAL("TCP attack should have port!");
					return std::nullopt;
				}
			}
			else if(*method == "udp")
			{
				SPDLOG_ERROR("Not implemented yet");
				return std::nullopt;
			}
			else
			{
				SPDLOG_ERROR("Failed to parse: {} to known methods!", *method);
				return std::nullopt;
			}
		}
		else
		{
			SPDLOG_ERROR("You should provide attack method like this method://target.com");
			return std::nullopt;
		}
	}

	attackTactic.squadSize = 
		std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();

	if(cmd.isDefined("--threads"))
	{
		const std::string threadsString = cmd.value("--threads");
		try
		{
			attackTactic.squadSize = std::stoull(threadsString);
		}
		catch(...)
		{
			SPDLOG_ERROR("Failed to parse {} to number", threadsString);
			return std::nullopt;
		}
	}

	return attackTactic;
}

