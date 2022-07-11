#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <netdb.h>
#include <netinet/ip.h>

#include <optional>

#include "uri.h"

namespace NetUtil
{
	struct SAddrInfoDeleter
	{
		void operator()(addrinfo *pAddrInfo)
		{
			freeaddrinfo(pAddrInfo);
		}
	};

	using CAddrInfo = std::unique_ptr<addrinfo[], SAddrInfoDeleter>;


	std::optional<CAddrInfo> GetHostAddresses(const CURI& cURIToGetAddress) noexcept;

	inline static constexpr const size_t IP_HEADER_LENGTH{20};
	using IPTCPPacket = std::array<char, IP_MAXPACKET>;

	std::uint16_t GenerateIPChecksum(uint16_t *addr, int len) noexcept;
} // NetUtil 

#endif // NET_UTILS_H
