#ifndef SETUP_H
#define SETUP_H

#include <optional>

#include "args-parser/all.hpp"

#include "target.hpp"
namespace Setup
{
	bool SetupArgs(Args::CmdLine &cmd);

	std::optional<Attackers::Tactic> GetTactic(const Args::CmdLine &cmd);
}

#endif // SETUP_H
