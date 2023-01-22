#ifndef UDP_WRAPPER
#define UDP_WRAPPER

#include <iostream>
#include <netinet/ip.h>

#include <array>

#include "module.h"
#include "net_utils.h"
#include "resolvers.h"
#include "uri.h"

namespace Wrappers::UDP
{

class UDPModule : public IModule
{
public:
	UDPModule() = default;
	~UDPModule() override {}

	bool Initialize() override
	{
		return true;
	}
};

class UDPWrapper
{
public:
	UDPWrapper(NetUtil::PAddressResolver resolver);
	~UDPWrapper();

	UDPWrapper(const UDPWrapper &copy) = delete;
	UDPWrapper& operator=(const UDPWrapper &copy) = delete;

	UDPWrapper(UDPWrapper &&move) = default;
	UDPWrapper& operator=(UDPWrapper &&move) = default;

	bool SendPacket(const URI &srcAddress, const URI &destAddress);

private:
	void CreatePacket(const URI &srcAddress, const URI &destAddress);

private:
	NetUtil::IPPacket m_currentPacket;
	sockaddr_in m_sin;

	int m_socketHandle;

	NetUtil::PAddressResolver m_resolver;
};

class UDPAddressResolver : public NetUtil::AddressResolver
{
public:
	NetUtil::PossibleAddress ResolveHostAddress(const URI &hostAddress) override;
};
}


#endif // UDP_WRAPPER 
