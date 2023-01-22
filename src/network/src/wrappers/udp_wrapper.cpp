#include "resolvers.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define __FAVOR_BSD
#include <netinet/udp.h>

#include "net_utils.h"
#include "spdlog/spdlog.h"

#include "udp_wrapper.h"
#include "utils.h"

using namespace Wrappers::UDP;
using namespace NetUtil;

UDPWrapper::UDPWrapper(PAddressResolver resolver) :
	m_resolver{resolver}
{
	m_socketHandle = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(m_socketHandle == -1)
	{
		SPDLOG_ERROR("Error while creating UDP socket, check if you are running with root");

		// Throwing now, because we cannot fix or handle this
		throw std::runtime_error("Error while creating UDP socket, check if you are running with root");
	}
}

UDPWrapper::~UDPWrapper()
{
	if(m_socketHandle != -1)
	{
		close(m_socketHandle);
	}
}

struct UDPPseudoHeader
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t udp_length;
};

void UDPWrapper::CreatePacket(const URI &srcAddress, const URI &destAddress)
{
	std::fill(std::begin(m_currentPacket), std::end(m_currentPacket), 0);

	// IP header
	struct iphdr &ipHeader = 
		*reinterpret_cast<struct iphdr *>(m_currentPacket.data());
	// UDP header
	struct udphdr &udpHeader = 
		*reinterpret_cast<struct udphdr *>(m_currentPacket.data() +
			sizeof(struct ip));

	FillWithRandom(
		std::begin(m_currentPacket) + IP_HEADER_LENGTH + UDP_HEADER_LENGTH, 
		std::end(m_currentPacket));

	m_sin.sin_family = AF_INET;
	m_sin.sin_port = htons(destAddress.GetPort().value_or(80));
	m_sin.sin_addr.s_addr = inet_addr(destAddress.GetPureAddress().c_str());

	// Setting IP header
	ipHeader.ihl = 5;
	ipHeader.version = 4;
	ipHeader.tos = 0;
	ipHeader.tot_len = m_currentPacket.size();
	ipHeader.id = htonl(54321);
	ipHeader.frag_off = 0;
	ipHeader.ttl = 225;
	ipHeader.protocol = IPPROTO_UDP;
	ipHeader.check = 0;
	ipHeader.saddr = inet_addr(srcAddress.GetPureAddress().c_str());
	ipHeader.daddr = m_sin.sin_addr.s_addr;

	ipHeader.check = GenerateIPChecksum(reinterpret_cast<uint16_t *>(m_currentPacket.data()), 
		IP_HEADER_LENGTH);

	// Setting UDP header
	udpHeader.source = htons(srcAddress.GetPort().value_or(80));
	udpHeader.dest = htons(destAddress.GetPort().value_or(80));
	udpHeader.len = htons(m_currentPacket.size() - sizeof(ipHeader));
	// On Linux checksum is added automaticaly
	udpHeader.check = 0;
}

bool UDPWrapper::SendPacket(const URI &srcAddress, const URI &destAddress)
{
	const auto resolvedAddress{m_resolver->ResolveHostAddress(destAddress)};
	if(!resolvedAddress)
	{
		return false;
	}

	CreatePacket(srcAddress, *resolvedAddress);
	int error = sendto(m_socketHandle, m_currentPacket.data(), 
		m_currentPacket.size(), 0, reinterpret_cast<struct sockaddr *>(&m_sin),
		sizeof(m_sin));

	if(error < 0)
	{
		SPDLOG_ERROR("Failed to send UDP datagram, got error: {}", std::strerror(error));
	}

	return error > 0;
}

PossibleAddress UDPAddressResolver::ResolveHostAddress(const URI &hostAddress)
{
	addrinfo addressHint;
	std::memset(&addressHint, 0, sizeof(addressHint));
	addressHint.ai_family = AF_INET;
	addressHint.ai_socktype = SOCK_DGRAM;
	addressHint.ai_protocol = IPPROTO_UDP;

	return NetUtil::ResolveHostAddressByAddrInfo(hostAddress, addressHint);
}
