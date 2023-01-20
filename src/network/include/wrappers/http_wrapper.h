#ifndef CURL_WRAPPER_HPP
#define CURL_WRAPPER_HPP

#include <memory>
#include <mutex>
#include <string>
#include <optional>

#include "curl/curl.h"

#include "api_interface.h"
#include "http_structs.hpp"
#include "curl_utils.h"

namespace HTTP
{

class HTTPWrapper
{
public:
	HTTPWrapper();

	HTTPWrapper(HTTPWrapper &&moveLoader) noexcept = default;
	HTTPWrapper &operator=(HTTPWrapper &&moveLoader) noexcept = default;

	HTTPWrapper(const HTTPWrapper &copyLoader) = delete;
	HTTPWrapper &operator=(const HTTPWrapper &copyLoader) = delete;

	void SetTarget(const std::string &address);
	void SetProxy(const Proxy &proxy);
	void SetHeaders(const Headers &headers);

	std::optional<Response> Download(long timeout=5);
	std::optional<HTTPCode> Ping(long timeout=5);
	std::optional<Response> Send(const Payload &payload, long timeout=5);
private:
	void SetDefaultOptions();

private:
	CURL_UTILS::PCURL m_curlEnv;
	CURL_UTILS::UseHeadersConfig m_headersConfig;
};

}

#endif // CURL_WRAPPER_HPP
