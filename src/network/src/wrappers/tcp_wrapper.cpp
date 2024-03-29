#include "proxy_checker.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h> 
#define __FAVOR_BSD     
#include <netinet/tcp.h>
#include <arpa/inet.h>  

#include <errno.h>      
#include <string>

#include "tcp_wrapper.h"
#include "http_wrapper.h"
#include "net_utils.h"
#include "config.h"

using namespace Wrappers::TCP;

TCPWrapper::TCPWrapper(NetUtil::PAddressResolver resolver) :
	m_resolver{resolver}
{
	// Firstly probing for interface
	m_socketFD = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(setsockopt(m_socketFD, IPPROTO_IP, IP_HDRINCL, &ON, sizeof(ON)) < 0)
	{
		close(m_socketFD);
		m_socketFD = -1;
		SPDLOG_ERROR("Error while creating TCP socket, check if you are running with root");

		// Throwing now, because we cannot fix or handle this
		throw std::runtime_error("Error while creating TCP socket, check if you are running with root");
	}
}

TCPWrapper::~TCPWrapper() noexcept
{
	if(m_socketFD != -1)
	{
		close(m_socketFD);
	}
}

bool TCPWrapper::SendConnectPacket(const URI &srcAddress, const URI &destAddress) noexcept
{
	if(m_socketFD == -1)
	{
		SPDLOG_ERROR("Error while sending TCP packet. Socket is not created, check if you are running with root");
		return false;
	}

	if(!CreatePacket(srcAddress, destAddress))
	{
		return false;
	}

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(destAddress.GetPort().value_or(80));
	addr.sin_addr.s_addr = inet_addr(destAddress.GetPureAddress().c_str());

	if(sendto(m_socketFD, m_currentPacket.data(),
		NetUtil::IP_HEADER_LENGTH + TCP_HEADER_LENGTH, 0, 
		reinterpret_cast<struct sockaddr *>(&addr),
		sizeof(addr)) < 0)
	{
		SPDLOG_INFO("Got error while sending package: {}", std::strerror(errno));
		return false;
	}

	return true;
}

bool TCPWrapper::CreatePacket(const URI &srcAddress, const URI &destAddress) noexcept
{
	// Headers for IP
	struct ip ipHeader;
	std::memset(&ipHeader, 0, sizeof(ipHeader));
	ipHeader.ip_hl = NetUtil::IP_HEADER_LENGTH / sizeof (uint32_t);
	// Internet Protocol version (4 bits): IPv4
	ipHeader.ip_v = 4;
	// Type of service (8 bits)
	ipHeader.ip_tos = 0;
	// Total length of datagram (16 bits): IP header + TCP header
	ipHeader.ip_len = htons(NetUtil::IP_HEADER_LENGTH + TCP_HEADER_LENGTH);
	// ID sequence number (16 bits): unused, since single datagram
	ipHeader.ip_id = htons (0);
	// Time-to-Live (8 bits): default to maximum value
	ipHeader.ip_ttl = 255;
	// Transport layer protocol (8 bits): 6 for TCP
	ipHeader.ip_p = IPPROTO_TCP;

	int ipFlags[4];
	// Zero (1 bit)
	ipFlags[0] = 0;
	// Do not fragment flag (1 bit)
	ipFlags[1] = 0;
	// More fragments following flag (1 bit)
	ipFlags[2] = 0;
	// Fragmentation offset (13 bits)
	ipFlags[3] = 0;
	ipHeader.ip_off = htons((ipFlags[0] << 15)
						  + (ipFlags[1] << 14)
						  + (ipFlags[2] << 13)
						  +  ipFlags[3]);

	int respCode = inet_pton(AF_INET, srcAddress.GetPureAddress().c_str(), &(ipHeader.ip_src));
	if(respCode == -1)
	{
		return false;
	}
	respCode = inet_pton(AF_INET, destAddress.GetPureAddress().c_str(), &(ipHeader.ip_dst));
	if(respCode == -1)
	{
		return false;
	}

	// IPv4 header checksum (16 bits): set to 0 when calculating checksum
	ipHeader.ip_sum = 0;
	ipHeader.ip_sum = NetUtil::GenerateIPChecksum((uint16_t *) &ipHeader, NetUtil::IP_HEADER_LENGTH);

	// TCP header
	struct tcphdr tcpHeader;
	std::memset(&tcpHeader, 0, sizeof(tcpHeader));
	// Source port number (16 bits)
	tcpHeader.th_sport = htons(60);

	// Destination port number (16 bits)
	tcpHeader.th_dport = htons(80);

	// Sequence number (32 bits)
	tcpHeader.th_seq = htonl(0);

	// Acknowledgement number (32 bits): 0 in first packet of SYN/ACK process
	tcpHeader.th_ack = htonl(0);

	// Reserved (4 bits): should be 0
	tcpHeader.th_x2 = 0;

	// Data offset (4 bits): size of TCP header in 32-bit words
	tcpHeader.th_off = TCP_HEADER_LENGTH / 4;

	int tcpFlags[8];
	// FIN flag (1 bit)
	tcpFlags[0] = 0;
	// SYN flag (1 bit): set to 1
	tcpFlags[1] = 1;
	// RST flag (1 bit)
	tcpFlags[2] = 0;
	// PSH flag (1 bit)
	tcpFlags[3] = 0;
	// ACK flag (1 bit)
	tcpFlags[4] = 0;
	// URG flag (1 bit)
	tcpFlags[5] = 0;
	// ECE flag (1 bit)
	tcpFlags[6] = 0;
	// CWR flag (1 bit)
	tcpFlags[7] = 0;

	tcpHeader.th_flags = 0;
	for (int i = 0; i < 8; i++) {
		tcpHeader.th_flags += (tcpFlags[i] << i);
	}
	// Window size (16 bits)
	tcpHeader.th_win = htons(65535);
	// Urgent pointer (16 bits): 0 (only valid if URG flag is set)
	tcpHeader.th_urp = htons(0);
	// TCP checksum (16 bits)
	tcpHeader.th_sum = GenerateTCPChecksum(ipHeader, tcpHeader);

	// Setting IPv4 hedaer to packet
	std::memcpy(m_currentPacket.data(), &ipHeader, NetUtil::IP_HEADER_LENGTH * sizeof (uint8_t));
	// Setting TCP header
	std::memcpy(
		m_currentPacket.data() + NetUtil::IP_HEADER_LENGTH,
		&tcpHeader,
		TCP_HEADER_LENGTH * sizeof (uint8_t));

	return true;
}

std::uint16_t TCPWrapper::GenerateTCPChecksum(struct ip iphdr, struct tcphdr tcphdr) noexcept
{
	std::uint16_t svalue;
	char buf[IP_MAXPACKET], cvalue;
	char *ptr;
	int chksumlen = 0;

	// ptr points to beginning of buffer buf
	ptr = &buf[0];

	// Copy source IP address into buf (32 bits)
	std::memcpy(ptr, &iphdr.ip_src.s_addr, sizeof(iphdr.ip_src.s_addr));
	ptr += sizeof(iphdr.ip_src.s_addr);
	chksumlen += sizeof(iphdr.ip_src.s_addr);

	// Copy destination IP address into buf (32 bits)
	std::memcpy(ptr, &iphdr.ip_dst.s_addr, sizeof(iphdr.ip_dst.s_addr));
	ptr += sizeof(iphdr.ip_dst.s_addr);
	chksumlen += sizeof(iphdr.ip_dst.s_addr);

	// Copy zero field to buf (8 bits)
	*ptr = 0; ptr++;
	chksumlen += 1;

	// Copy transport layer protocol to buf (8 bits)
	std::memcpy(ptr, &iphdr.ip_p, sizeof(iphdr.ip_p));
	ptr += sizeof(iphdr.ip_p);
	chksumlen += sizeof(iphdr.ip_p);

	// Copy TCP length to buf (16 bits)
	svalue = htons(sizeof(tcphdr));
	std::memcpy(ptr, &svalue, sizeof(svalue));
	ptr += sizeof(svalue);
	chksumlen += sizeof(svalue);

	// Copy TCP source port to buf (16 bits)
	std::memcpy(ptr, &tcphdr.th_sport, sizeof(tcphdr.th_sport));
	ptr += sizeof(tcphdr.th_sport);
	chksumlen += sizeof(tcphdr.th_sport);

	// Copy TCP destination port to buf (16 bits)
	std::memcpy(ptr, &tcphdr.th_dport, sizeof(tcphdr.th_dport));
	ptr += sizeof(tcphdr.th_dport);
	chksumlen += sizeof(tcphdr.th_dport);

	// Copy sequence number to buf (32 bits)
	std::memcpy(ptr, &tcphdr.th_seq, sizeof(tcphdr.th_seq));
	ptr += sizeof(tcphdr.th_seq);
	chksumlen += sizeof(tcphdr.th_seq);

	// Copy acknowledgement number to buf (32 bits)
	std::memcpy(ptr, &tcphdr.th_ack, sizeof(tcphdr.th_ack));
	ptr += sizeof(tcphdr.th_ack);
	chksumlen += sizeof (tcphdr.th_ack);

	// Copy data offset to buf (4 bits) and
	// copy reserved bits to buf (4 bits)
	cvalue = (tcphdr.th_off << 4) + tcphdr.th_x2;
	std::memcpy(ptr, &cvalue, sizeof(cvalue));
	ptr += sizeof (cvalue);
	chksumlen += sizeof (cvalue);

	// Copy TCP flags to buf (8 bits)
	std::memcpy(ptr, &tcphdr.th_flags, sizeof(tcphdr.th_flags));
	ptr += sizeof (tcphdr.th_flags);
	chksumlen += sizeof (tcphdr.th_flags);

	// Copy TCP window size to buf (16 bits)
	std::memcpy(ptr, &tcphdr.th_win, sizeof(tcphdr.th_win));
	ptr += sizeof (tcphdr.th_win);
	chksumlen += sizeof(tcphdr.th_win);

	// Copy TCP checksum to buf (16 bits)
	// Zero, since we don't know it yet
	*ptr = 0;
	ptr++;
	*ptr = 0;
	ptr++;

	chksumlen += 2;

	// Copy urgent pointer to buf (16 bits)
	std::memcpy(ptr, &tcphdr.th_urp, sizeof(tcphdr.th_urp));
	ptr += sizeof(tcphdr.th_urp);
	chksumlen += sizeof(tcphdr.th_urp);

	return NetUtil::GenerateIPChecksum((uint16_t *) buf, chksumlen);
}

NetUtil::PossibleAddress TCPAddressResolver::ResolveHostAddress(const URI &hostAddress)
{
	addrinfo addressHint;
	std::memset(&addressHint, 0, sizeof(addressHint));
	addressHint.ai_family = AF_INET;
	addressHint.ai_socktype = SOCK_STREAM;
	addressHint.ai_protocol = IPPROTO_TCP;

	return NetUtil::ResolveHostAddressByAddrInfo(hostAddress, addressHint);
}
