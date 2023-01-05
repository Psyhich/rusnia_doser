#ifndef UDP_GUN
#define UDP_GUN

#include "gun.hpp"
#include "wrappers/udp_wrapper.h"

namespace Attackers
{
	class UDPGun : public IGun
	{
	public:
		UDPGun(const TaskController &task) : IGun(task)
		{}
		~UDPGun() override {}

		std::size_t FireTillDead(const URI &targetToKill) noexcept override;
	private:
		Wrappers::UDPWrapper m_flooder;
	};
}

#endif // UDP_GUN
