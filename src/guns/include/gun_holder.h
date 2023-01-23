#ifndef GUN_HOLDER_HPP
#define GUN_HOLDER_HPP

#include "gun.hpp"
#include "http_gun.h"
#include "inheritance_wrapper.hpp"
#include "tcp_gun.h"
#include "udp_gun.h"

namespace Attackers
{
class Gun : public
	InheritanceWrapper<IGun, std::max({sizeof(HTTPGun), sizeof(UDPGun), sizeof(TCPGun)})>
{
public:
	bool EmplaceGun(Attackers::AttackMethod attackMethod,
		const TaskController &owningTask, Wrappers::HTTP::SPProxyGetter proxyGetter,
		NetUtil::PAddressResolver tcpAddressResolver,
		NetUtil::PAddressResolver udpAdressResolver);
};
}

#endif // GUN_HOLDER_HPP
