#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

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

	inline static constexpr const int IP_PACKET_LENGTH{1500};

	inline static constexpr const int IP_HEADER_LENGTH{sizeof(struct iphdr)};
	inline static constexpr const int TCP_HEADER_LENGTH{sizeof(struct tcphdr)};
	inline static constexpr const int UDP_HEADER_LENGTH{sizeof(struct udphdr)};

	inline static constexpr const int IP_PACKET_DATA_SIZE{IP_PACKET_LENGTH - IP_HEADER_LENGTH};
	inline static constexpr const int UDP_DATA_SIZE{IP_PACKET_DATA_SIZE - UDP_HEADER_LENGTH};
	inline static constexpr const int TCP_DATA_SIZE{IP_PACKET_DATA_SIZE - TCP_HEADER_LENGTH};

	using IPPacket = std::array<char, IP_PACKET_LENGTH>;

	std::uint16_t GenerateIPChecksum(uint16_t *addr, int len) noexcept;
} // NetUtil 

#endif // NET_UTILS_H
