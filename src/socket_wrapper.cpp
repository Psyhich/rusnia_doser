#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <cstring>

#include "socket_wrapper.h"

std::optional<SocketWrapper::CAddrInfo> SocketWrapper::GetHostAddresses(const CURI& cURIToGetAddress) noexcept
{
	if(const auto cAddress = cURIToGetAddress.GetPureAddress())
	{
		// Setting hint to look for host(protocol, socket type and IPv4)
		addrinfo addressHint;
		std::memset(&addressHint, 0, sizeof(addressHint));
		addressHint.ai_family = AF_INET;
		addressHint.ai_socktype = SOCK_STREAM;
		addressHint.ai_protocol = 0;

		const std::string csService = 
			cURIToGetAddress.GetProtocol().value_or("");

		// Creating pointer for array of resolved hosts(we would need only first one)
		addrinfo *pResolvedHosts = nullptr;
		if(getaddrinfo(cAddress->c_str(), csService.c_str(), &addressHint, &pResolvedHosts) != 0 || 
			pResolvedHosts == nullptr)
		{
			fprintf(stderr, "Failed to resolve given address\n");
			return std::nullopt;
		}
		return CAddrInfo(pResolvedHosts);
	}
	return std::nullopt;
}

SocketWrapper::SocketWrapper(SocketType type) noexcept :
	m_type{type}
{
	m_socketFD = socket(AF_INET, type, 0);

}

SocketWrapper::~SocketWrapper() noexcept
{
	if(m_socketFD != -1)
	{
		close(m_socketFD);
	}
}

SocketWrapper::ConnectResponse SocketWrapper::Connect(const std::string &hostToConnect) noexcept
{
	const CURI uriToConnect{hostToConnect};

	const int port = htons(uriToConnect.GetPort().value_or(80));

	if(auto hostAddress{GetHostAddresses(uriToConnect)})
	{
		// We do a little CONNECT until it 500)
		bool allTimeout = true;
		for(addrinfo *pAddr = hostAddress->get(); 
			pAddr != nullptr; pAddr = pAddr->ai_next)
		{
			sockaddr_in address = 
				*reinterpret_cast<sockaddr_in *>(pAddr->ai_addr);
			address.sin_port = port;

			int resp = connect(m_socketFD, reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr));
			if(resp == 0)
			{
				m_connectedAddress = address.sin_addr.s_addr;
				return ConnectResponse::ConnectionSuccessful;
			}
			else if(resp == ETIMEDOUT)
			{
				allTimeout &= true;
			}
			else
			{
				allTimeout &= false;
			}
		}

		if(allTimeout)
		{
			return ConnectResponse::ConnectionTimeout;
		}
	}
	return ConnectResponse::ConnectionError;
}

