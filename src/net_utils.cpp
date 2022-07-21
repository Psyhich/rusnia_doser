#include <cstring>
#include <iostream>

#include "net_utils.h"

using namespace NetUtil;

std::optional<CAddrInfo> NetUtil::GetHostAddresses(const CURI& cURIToGetAddress) noexcept
{
	if(const auto cAddress = cURIToGetAddress.GetPureAddress())
	{
		// Setting hint to look for host(protocol, socket type and IPv4)
		addrinfo addressHint;
		std::memset(&addressHint, 0, sizeof(addressHint));
		addressHint.ai_family = AF_INET;
		addressHint.ai_socktype = SOCK_STREAM;
		addressHint.ai_protocol = 0;

		// Creating pointer for array of resolved hosts(we would need only first one)
		addrinfo *pResolvedHosts = nullptr;
		if(getaddrinfo(cAddress->c_str(), nullptr, &addressHint, &pResolvedHosts) != 0 || 
			pResolvedHosts == nullptr)
		{
			SPDLOG_WARN("Failed to resolve {}", cURIToGetAddress);
			return std::nullopt;
		}
		return CAddrInfo(pResolvedHosts);
	}
	return std::nullopt;
}


std::uint16_t NetUtil::GenerateIPChecksum(uint16_t *addr, int bytesCount) noexcept
{
	uint32_t sum = 0;
	int count = bytesCount;
	uint16_t answer = 0;

	// Sum up 2-byte values until none or only one byte left.
	while (count > 1)
	{
		sum += *(addr++);
		count -= 2;
	}

	// Add left-over byte, if any.
	if (count > 0)
	{
		sum += *(uint8_t *) addr;
	}

	// Fold 32-bit sum into 16 bits; we lose information by doing this,
	// increasing the chances of a collision.
	// sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

	// Checksum is one's compliment of sum.
	answer = ~sum;

	return answer;
}

