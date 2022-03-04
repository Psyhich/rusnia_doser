
#include "attacker.h"


void TCPFire(const std::vector<std::string> &apiList, std::atomic<bool> &shouldStop) noexcept
{
	// Algo
	// 1 create N sockets and give them address and port of target
	// 2 give them command to connect to target
	// 3 wait and poll for disconnected sockets to reconnect them again


}
