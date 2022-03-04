#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H

#include <sys/socket.h>
#include <netdb.h>

#include <string>
#include <optional>
#include <memory>

#include "uri.h"

class SocketWrapper
{
	struct SAddrInfoDeleter
	{
		void operator()(addrinfo *pAddrInfo)
		{
			freeaddrinfo(pAddrInfo);
		}
	};
	using CAddrInfo = std::unique_ptr<addrinfo[], SAddrInfoDeleter>;

	enum class ConnectResponse
	{
		ConnectionSuccessful,
		ConnectionTimeout,
		ConnectionError
	};
public:
	enum SocketType
	{
		TCPSocket = SOCK_STREAM,
		UDPSocket = SOCK_DGRAM
 	};

	explicit SocketWrapper(SocketType type) noexcept;
	~SocketWrapper() noexcept;

	ConnectResponse Connect(const std::string &hostToConnect) noexcept;
	bool Reconnect() noexcept;

	void SetTimeout(unsigned seconds) noexcept;

	inline int GetFD() noexcept
	{
		return m_socketFD;
	}
private:
	std::optional<CAddrInfo> GetHostAddresses(const CURI& cURIToGetAddress) noexcept;

private:
	SocketType m_type;
	std::string m_connectedAddress;
	int m_socketFD{-1};
};

#endif // SOCKET_WRAPPER_H
