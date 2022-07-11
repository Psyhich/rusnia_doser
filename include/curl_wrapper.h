#ifndef CURL_WRAPPER_HPP
#define CURL_WRAPPER_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <iostream>
#include <optional>

#include "curl/curl.h"
#include "response.hpp"

inline static std::once_flag g_curlInitFlag;

struct CURLDeleter
{
	void operator()(CURL *curlEnv) noexcept
	{
		curl_easy_cleanup(curlEnv);
	}
};
using PCURL = std::unique_ptr<CURL, CURLDeleter>;

using Headers = std::map<std::string, std::string>;

using Proxy = std::pair<std::string, std::string>;

class CURLLoader
{
public:
	CURLLoader() noexcept;

	CURLLoader(CURLLoader &&moveLoader) noexcept = default;
	CURLLoader &operator=(CURLLoader &&moveLoader) noexcept = default;

	CURLLoader(const CURLLoader &copyLoader) = delete;
	CURLLoader &operator=(const CURLLoader &copyLoader) = delete;

	void SetTarget(const std::string &address) noexcept;
	void SetProxy(const std::string &proxyIP, const std::string &proxyAuth) noexcept;
	void SetHeaders(const Headers &headers) noexcept;

	std::optional<Response> Download(long timeout=5) noexcept;
	std::optional<long> Ping(long timeout=5, Headers* headers = nullptr) noexcept;

	inline static const Headers BASE_HEADERS
		{
			{"Content-Type", "*/*"},
			{"Connection", "keep-alive"},
			{"Accept", "application/json, text/plain, */*"},
			{"Accept-Language", "ru"},
			{"Accept-Encoding", "gzip, deflate, br"},
			{"Cache-Control", "no-store"}
		};
private:
	static size_t DataCallback(void *contents, size_t size, size_t nmemb, void *userp) noexcept;
	static size_t DoNothingCallback(void *, size_t nmemb, size_t size, void *) noexcept
	{
		return size * nmemb;
	}

	static size_t ProcessHeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata);

	static size_t DoNothingWithHeader(char *, size_t size, size_t nitems, void *) noexcept
	{
		return size * nitems;
	}
	
	static void TrimString(std::string &strinToTrim);


	inline static bool ProcessCode(CURLcode code) noexcept
	{
		return code == CURLE_OK;
	}
private:
	PCURL m_curlEnv;
};

#endif // CURL_WRAPPER_HPP
