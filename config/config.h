#ifndef DOSER_CONFIG_H
#define DOSER_CONFIG_H

#include <cstddef>

namespace AttackerConfig 
{
	inline static constexpr const char *APIS_LIST = "https://gitlab.com/cto.endel/atack_hosts/-/raw/master/hosts.json";
	inline static constexpr const size_t FIRE_TIMEOUT_SECONDS = 10;
	inline static constexpr const size_t DISCOVER_TIMEOUT_SECONDS = 30;
	inline static constexpr const size_t MAX_ATTACK_ERRORS_COUNT = 10;

	namespace TCPAttacker
	{
		inline static constexpr const size_t TCP_ATTACKS_BEFORE_CHECK = 500;
		inline static constexpr const size_t PROXY_RETRIES = 2;
	}
}

// Proxy
namespace ProxyConfig
{
	inline static constexpr const char *PROXY_PROBING = "www.google.com";
	inline static constexpr const unsigned PROXY_TRIES = 2;
	constexpr const size_t MAX_PROXY_ATTACKS = 5000;
	constexpr const size_t MAX_BAD_PROXY_LOADS = 100;
} // ProxyConfig 

namespace {
	
}

#endif // DOSER_CONFIG_H
