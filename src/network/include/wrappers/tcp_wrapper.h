#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H

#include <netinet/ip.h>

#include <cstdint>
#include <optional>

#include "api_interface.h"
#include "module.h"
#include "net_utils.h"
#include "uri.h"
#include "resolvers.h"

namespace Wrappers::TCP
{

class TCPModule : public IModule
{
public:
	TCPModule() = default;
	~TCPModule() override {}

	bool Initialize() override
	{
		return true;
	}
};

class TCPWrapper
{
public:
	TCPWrapper(NetUtil::PAddressResolver resolver);
	~TCPWrapper() noexcept;

	bool SendConnectPacket(const URI &srcAddress, 
		const URI &destAddress) noexcept;

private:
	bool CreatePacket(const URI &srcAddress, 
		const URI &destAddress) noexcept;

	static std::uint16_t GenerateTCPChecksum(struct ip iphdr, struct tcphdr tcphdr) noexcept;
private:
	inline static const int ON{1};
	inline static constexpr const size_t TCP_HEADER_LENGTH{20};

	NetUtil::IPPacket m_currentPacket;
	NetUtil::PAddressResolver m_resolver;
	int m_socketFD{-1};
};

class TCPAddressResolver : public NetUtil::AddressResolver
{
public:
	TCPAddressResolver() = default;
	~TCPAddressResolver()
	{}

	NetUtil::PossibleAddress ResolveHostAddress(const URI &hostAddress) override;
};

};

#endif // SOCKET_WRAPPER_H
