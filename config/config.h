#ifndef DOSER_CONFIG_H
#define DOSER_CONFIG_H

#include <cstddef>

namespace AttackerConfig 
{
	inline static constexpr const char *APIS_LIST = "https://hutin-puy.nadom.app/hosts.json";

	inline static constexpr const std::size_t FIRE_TIMEOUT_SECONDS = 2;
	inline static constexpr const std::size_t DISCOVER_TIMEOUT_SECONDS = 15;
	inline static constexpr const std::size_t MAX_ATTACK_ERRORS_COUNT = 10;

	namespace TCPAttacker
	{
		inline static constexpr const std::size_t TCP_ATTACKS_BEFORE_CHECK = 100000;
		inline static constexpr const std::size_t MAX_ERRORS_BEFORE_CHECK = 10;
	}

	namespace UDPAttacker
	{
		inline static constexpr const std::size_t MAX_ATTACKS_BEFORE_CHECK = 100000;
	}
}

// Proxy
namespace ProxyConfig
{
	inline static constexpr const char *PROXY_PROBING = "www.google.com";
	inline static constexpr const unsigned PROXY_PROBING_TRIES = 2;
	inline static constexpr const unsigned PROXY_LOAD_TRIES = 2;
	constexpr const std::size_t MAX_PROXY_ATTACKS = 5000;
	constexpr const std::size_t MAX_BAD_PROXY_LOADS = 100;
}

#endif // DOSER_CONFIG_H
