#ifndef UDP_GUN
#define UDP_GUN

#include "gun.hpp"
#include "wrappers/udp_wrapper.h"

namespace Attackers
{
	class UDPGun : public IGun
	{
	public:
		UDPGun(const TaskController &task, NetUtil::PAddressResolver udpAdressResolver) :
			IGun(task),
			m_flooder{udpAdressResolver}
		{}
		~UDPGun() override {}

		std::size_t FireTillDead(const URI &targetToKill) override;
	private:
		Wrappers::UDPWrapper m_flooder;
	};
}

#endif // UDP_GUN
