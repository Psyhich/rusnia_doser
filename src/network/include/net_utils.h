#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <map>
#include <optional>

#include "uri.h"

namespace NetUtil
{
	using Headers = std::map<std::string, std::string>;

	struct SAddrInfoDeleter
	{
		void operator()(addrinfo *pAddrInfo)
		{
			freeaddrinfo(pAddrInfo);
		}
	};

	using CAddrInfo = std::unique_ptr<addrinfo[], SAddrInfoDeleter>;


	// TODO: rework this function to apply
	// more filters to get only one return address
	std::optional<std::string> ResolveHostAddressByAddrInfo(const URI &hostAddress,
		const addrinfo &addressInfo);

	inline static constexpr const int IP_PACKET_LENGTH{1500};

	inline static constexpr const int IP_HEADER_LENGTH{sizeof(struct iphdr)};
	inline static constexpr const int TCP_HEADER_LENGTH{sizeof(struct tcphdr)};
	inline static constexpr const int UDP_HEADER_LENGTH{sizeof(struct udphdr)};

	inline static constexpr const int IP_PACKET_DATA_SIZE{IP_PACKET_LENGTH - IP_HEADER_LENGTH};
	inline static constexpr const int UDP_DATA_SIZE{IP_PACKET_DATA_SIZE - UDP_HEADER_LENGTH};
	inline static constexpr const int TCP_DATA_SIZE{IP_PACKET_DATA_SIZE - TCP_HEADER_LENGTH};

	using IPPacket = std::array<char, IP_PACKET_LENGTH>;

	std::uint16_t GenerateIPChecksum(uint16_t *addr, int len) noexcept;

	std::string ChoseUseragent();

	inline void UpdateHeaders(Headers &headers,
		const std::string &proxyIP) noexcept
	{
		headers["User-Agent"] = ChoseUseragent();
		headers["X-Forwarder-For"] = proxyIP;
		headers["Cf-Visitor"] = "https";
		headers["X-Forwarded-Proto"]  = "https";
	}

	std::string DecodeURL(const std::string &stringToDecode);

	std::string GetRandomIP();
	int GetRandomPort() noexcept;

} // NetUtil 

#endif // NET_UTILS_H
