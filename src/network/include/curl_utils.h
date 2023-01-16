#ifndef CURL_UTILS_H
#define CURL_UTILS_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "curl/multi.h"
#include "curl/curl.h"
#include "http_structs.hpp"

namespace HTTP::CURL_UTILS
{
struct CURLDeleter
{
	void operator()(CURL *curlEnv)
	{
		curl_easy_cleanup(curlEnv);
	}
};
using PCURL = std::unique_ptr<CURL, CURLDeleter>;

struct CURLMultiDeleter
{
	void operator()(CURLM *curlEnv)
	{
		curl_multi_cleanup(curlEnv);
	}
};
using PCURLM = std::unique_ptr<CURLM, CURLMultiDeleter>;

struct CURLListDeleter
{
	void operator()(curl_slist *listToDeallocate)
	{
		curl_slist_free_all(listToDeallocate);
	}
};
using PCURLList = std::unique_ptr<curl_slist, CURLListDeleter>;

class CURLInitializer
{
public:
	CURLInitializer();
	~CURLInitializer();
};
inline static CURLInitializer m_curlInitializer;

struct ReadBytesCounter
{
	ReadBytesCounter(const std::vector<std::uint8_t> &bytes) :
		m_bytes{bytes}
	{}

	const std::vector<std::uint8_t> &m_bytes;
	std::size_t bytesRead{0};
};

// TODO: rewrite this by using creating curl
// config interface and using virtual method apply
class SaveHeadersConfig
{
public:
	SaveHeadersConfig(CURL *curlEnv, Headers &headersTarget);
	~SaveHeadersConfig();
private:
	static size_t SaveToHeaderCallback(char *buffer, size_t size, size_t nitems, void *headersPtr);

private:
	CURL *m_curlEnv;
};
size_t DoNothingWithHeader(char *, size_t size, size_t nitems, void *);

struct DownloadConfig
{
public:
	DownloadConfig(CURL *curlEnv, std::vector<char> &dataTarget);
	~DownloadConfig();
private:
	static size_t SaveDataToVectorCallback(void *contents, size_t size, size_t nmemb, void *vectorPtr);

private:
	CURL *m_curlEnv;
};
size_t DoNothingWithDownloads(void *, size_t nmemb, size_t size, void *);

class UploadConfig
{
public:
	UploadConfig(CURL *curlEnv, const std::vector<std::uint8_t> &dataSource);
	~UploadConfig();
private:
	static size_t UploadVectorDataCallback(void *buffer, size_t size, size_t nitems, void *bytesCounterPtr);

private:
	CURL *m_curlEnv;
	ReadBytesCounter m_readBytesCounter;
};
size_t DoNothingWithUploads(void *buffer, size_t size, size_t nitems, void *bytesCounterPtr);

class UseHeadersConfig
{
public:
	UseHeadersConfig(CURL *curlEnv, const Headers &headersToUse);

	UseHeadersConfig(const UseHeadersConfig &copyFrom) = delete;
	UseHeadersConfig &operator=(const UseHeadersConfig &copyFrom) = delete;

	UseHeadersConfig(UseHeadersConfig &&copyFrom) = default;
	UseHeadersConfig &operator=(UseHeadersConfig &&copyFrom) = default;

	void SetHeaders(const Headers &headersToUse);
private:
	CURL *m_curlEnv;
	PCURLList m_curlHeaders;
};

PCURLList HeadersToCurlSList(const Headers &headers);

bool ProcessCode(CURLcode code) noexcept;

void TrimString(std::string &strinToTrim);
};

#endif // CURL_UTILS_H
