#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H

#include <netinet/ip.h>

#include <cstdint>
#include <optional>

#include "api_interface.h"
#include "net_utils.h"
#include "proxy_checker.h"
#include "uri.h"


class TCPWrapper
{
public:
	enum class TCPStatus
	{
		GotError,
		NeedConnectivityCheck,
		Success
	};

	TCPWrapper() noexcept;
	~TCPWrapper() noexcept;

	TCPStatus SendConnectPacket(const URI &srcAddress, 
		const URI &destAddress) noexcept;

	std::optional<URI> TryResolveAddress(const URI &destAddress, 
		const NetUtil::ProxyList &proxies) noexcept;

	void SetTimeout(unsigned seconds) noexcept;

private:
	std::optional<NetUtil::IPPacket> CreatePacket(const URI &srcAddress, 
		const URI &destAddress) noexcept;
	std::uint16_t GenerateTCPChecksum(struct ip iphdr, struct tcphdr tcphdr) noexcept;
private:
	inline static const int ON{1};
	inline static constexpr const size_t TCP_HEADER_LENGTH{20};

	int m_socketFD{-1};
};

#endif // SOCKET_WRAPPER_H
