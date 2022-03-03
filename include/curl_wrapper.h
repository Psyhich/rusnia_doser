#ifndef CURL_WRAPPER_HPP
#define CURL_WRAPPER_HPP

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
	void operator()(CURL *curlEnv)
	{
		curl_easy_cleanup(curlEnv);
	}
};
using PCURL = std::unique_ptr<CURL, CURLDeleter>;

using Headers = std::map<std::string, std::string>;

class MultiLoader;

class CURLLoader
{
public:
	CURLLoader();

	CURLLoader(CURLLoader &&moveLoader) = default;
	CURLLoader &operator=(CURLLoader &&moveLoader) = default;

	CURLLoader(const CURLLoader &copyLoader) = delete;
	CURLLoader &operator=(const CURLLoader &copyLoader) = delete;

	void SetTarget(const std::string &address);
	void SetProxy(const std::string &proxyIP, const std::string &proxyAuth);
	void SetHeaders(const Headers &headers);

	std::optional<Response> Download(long timeout=5);
	long Ping(long timeout=5);

private:
	static size_t DataCallback(void *contents, size_t size, size_t nmemb, void *userp);
	inline static size_t DoNothingCallback(void *, size_t, size_t size, void *)
	{
		return size;
	}


	inline static bool ProcessCode(CURLcode code)
	{
		return code == CURLE_OK;
	}

	friend class MultiLoader;
private:
	PCURL m_curlEnv;
};

#endif // CURL_WRAPPER_HPP
