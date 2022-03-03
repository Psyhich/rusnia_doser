#ifndef DOSER_CONFIG_H
#define DOSER_CONFIG_H

#include <cstddef>

namespace AttackerConfig 
{
	inline static constexpr const char *APIS_LIST = "http://rockstarbloggers.ru/hosts.json";
	inline static constexpr const size_t FIRE_TIMEOUT_SECONDS = 10;
	inline static constexpr const size_t DISCOVER_TIMEOUT_SECONDS = 30;
}

// Proxy
namespace ProxyConfig
{
	inline static constexpr const char *PROXY_PROBING = "https://yandex.com/";
	inline static constexpr const unsigned PROXY_TRIES = 2;
	constexpr const size_t MAX_PROXY_ATTACKS = 5000;
	constexpr const size_t MAX_BAD_PROXY_LOADS = 100;
} // ProxyConfig 

namespace {
	
}

#endif // DOSER_CONFIG_H
