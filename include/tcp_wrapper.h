#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <net/if.h>     

#include <cstdint>
#include <string>
#include <optional>
#include <memory>

#include "gun.h"
#include "curl_wrapper.h"
#include "uri.h"


class TCPWrapper
{
struct SAddrInfoDeleter
{
	void operator()(addrinfo *pAddrInfo)
	{
		freeaddrinfo(pAddrInfo);
	}
};
using CAddrInfo = std::unique_ptr<addrinfo[], SAddrInfoDeleter>;

public:
	using IPTCPPacket = std::array<char, IP_MAXPACKET>;

	enum class TCPStatus
	{
		GotError,
		NeedConnectivityCheck,
		Success
	};

	TCPWrapper() noexcept;
	~TCPWrapper() noexcept;

	TCPStatus SendConnectPacket(const Attackers::Target &srcAddress, const Attackers::Target &destAddress) noexcept;

	std::optional<Attackers::Target> CheckConnection(const CURI &destAddress, const std::vector<Proxy> &proxies) noexcept;

	void SetTimeout(unsigned seconds) noexcept;

private:
	static std::optional<CAddrInfo> GetHostAddresses(const CURI& cURIToGetAddress) noexcept;
	static std::optional<IPTCPPacket> CreatePacket(const Attackers::Target &srcAddress, 
												const Attackers::Target &destAddress) noexcept;
	static std::uint16_t GenerateIPChecksum(uint16_t *addr, int len) noexcept;
	static std::uint16_t GenerateTCPChecksum(struct ip iphdr, struct tcphdr tcphdr) noexcept;
private:
	inline static const int ON{1};
	inline static constexpr const size_t IP_HEADER_LENGTH{20};
	inline static constexpr const size_t TCP_HEADER_LENGTH{20};

	int m_socketFD{-1};
};

#endif // SOCKET_WRAPPER_H
