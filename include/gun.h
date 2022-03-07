#ifndef GUN_H
#define GUN_H

#include <optional>
#include <string>

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
	virtual std::optional<Target> Aim(const CURI &uriToAttack) noexcept = 0;
	virtual void FireTillDead(const Target &targetToKill) noexcept = 0;
private:
};

} // Attackers

#endif // GUN_H
