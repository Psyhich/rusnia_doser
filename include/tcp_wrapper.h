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

#include "curl_wrapper.h"
#include "net_utils.h"
#include "target.hpp"
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

	TCPStatus SendConnectPacket(const CURI &srcAddress, 
		const CURI &destAddress) noexcept;

	std::optional<CURI> CheckConnection(const CURI &destAddress, 
		const std::vector<Proxy> &proxies) noexcept;

	void SetTimeout(unsigned seconds) noexcept;

private:
	std::optional<NetUtil::IPPacket> CreatePacket(const CURI &srcAddress, 
		const CURI &destAddress) noexcept;
	std::uint16_t GenerateTCPChecksum(struct ip iphdr, struct tcphdr tcphdr) noexcept;
private:
	inline static const int ON{1};
	inline static constexpr const size_t TCP_HEADER_LENGTH{20};

	int m_socketFD{-1};
};

#endif // SOCKET_WRAPPER_H
