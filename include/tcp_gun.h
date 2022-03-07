#ifndef TCP_GUN_H
#define TCP_GUN_H

#include "gun.h"

namespace Attackers
{

class TCPGun : IGun
{
public:
	std::optional<Target> Aim(const CURI &uriToAttack) noexcept override;
	void FireTillDead(const Target &targetToKill) noexcept override;
private:
};


} // Attackers 


#endif // TCP_GUN_H
