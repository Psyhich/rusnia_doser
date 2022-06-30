#ifndef GUN_H
#define GUN_H

#include <optional>
#include <string>

#include "multithread.h"
#include "uri.h"

namespace Attackers
{

struct Target
{
	std::string address;
	int port;
};

class IGun
{
public:
	IGun(const TaskController &task) : m_currentTask{task}
	{ }

	virtual std::optional<Target> Aim(const CURI &uriToAttack) noexcept = 0;
	virtual void FireTillDead(const Target &targetToKill) noexcept = 0;
protected:
	const TaskController &m_currentTask;
};

} // Attackers

#endif // GUN_H
