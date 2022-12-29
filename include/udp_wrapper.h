#ifndef UDP_WRAPPER
#define UDP_WRAPPER

#include <iostream>
#include <netinet/ip.h>

#include <array>

#include "net_utils.h"
#include "uri.h"

namespace Wrappers
{
	class UDPWrapper
	{
	public:
		UDPWrapper();
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
	};
}


#endif // UDP_WRAPPER 
