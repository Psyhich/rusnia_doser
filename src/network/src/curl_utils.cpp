#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "curl_utils.h"
#include "http_structs.hpp"

namespace HTTP::CURL_UTILS
{
CURLInitializer::CURLInitializer()
{
	if(!curl_global_init(CURL_GLOBAL_ALL))
	{
		std::runtime_error("Got error while doing global initializtion of curl functional");
	}
}
CURLInitializer::~CURLInitializer()
{
	curl_global_cleanup();
}

SaveHeadersConfig::SaveHeadersConfig(CURL *curlEnv, Headers &headersTarget) :
	m_curlEnv{curlEnv}
{
	curl_easy_setopt(m_curlEnv, CURLOPT_HEADERFUNCTION, SaveToHeaderCallback);
	curl_easy_setopt(m_curlEnv, CURLOPT_HEADERDATA, &headersTarget);
}
SaveHeadersConfig::~SaveHeadersConfig()
{
	curl_easy_setopt(m_curlEnv, CURLOPT_HEADERFUNCTION, DoNothingWithHeader);
	curl_easy_setopt(m_curlEnv, CURLOPT_HEADERDATA, nullptr);
}
size_t SaveHeadersConfig::SaveToHeaderCallback(char *buffer, size_t size, size_t nitems, void *headersPtr)
{
	char *currentChar = buffer;
	const size_t headerSize = size * nitems;

	std::string key;
	std::string value;

	size_t currentIndex{ 0 };
	bool foundColon = false;

	for(; currentIndex < headerSize; 
		currentChar++, ++currentIndex)
	{
		if(*currentChar == ':')
		{
			foundColon = true;
			break;
		}
		key.push_back(*currentChar);
	}

	if(!foundColon)
	{
		return headerSize;
	}

	++currentChar;
	++currentIndex;

	for(; currentIndex < headerSize; currentIndex++, currentChar++)
	{
		value.push_back(*currentChar);
	}

	TrimString(key);
	std::transform(std::begin(key), std::end(key), std::begin(key), tolower);
	key.shrink_to_fit();
	
	TrimString(value);
	value.shrink_to_fit();

	Headers &headers = *static_cast<Headers *>(headersPtr);
	headers.emplace(std::move(key), std::move(value));

	return size * nitems;
}
size_t DoNothingWithHeader(char *, size_t size, size_t nitems, void *)
{
	return size * nitems;
}

DownloadConfig::DownloadConfig(CURL *curlEnv, std::vector<char> &dataTarget) :
	m_curlEnv{curlEnv}
{
	curl_easy_setopt(m_curlEnv, CURLOPT_WRITEDATA, &dataTarget);
	curl_easy_setopt(m_curlEnv, CURLOPT_WRITEFUNCTION, SaveDataToVectorCallback);
}
DownloadConfig::~DownloadConfig()
{
	curl_easy_setopt(m_curlEnv, CURLOPT_WRITEDATA, nullptr);
	curl_easy_setopt(m_curlEnv, CURLOPT_WRITEFUNCTION, DoNothingWithDownloads);
}
size_t DownloadConfig::SaveDataToVectorCallback(void *contents, size_t size, size_t nmemb, void *vectorPtr)
{
	std::vector<char> &readBuffer = *static_cast<std::vector<char>*>(vectorPtr);

	readBuffer.reserve(readBuffer.size() + nmemb);

	for(size_t i = 0; i < nmemb; i++)
	{
		readBuffer.push_back(((char*)contents)[i]);
	}

	return nmemb * size;
}
size_t DoNothingWithDownloads(void *, size_t nmemb, size_t size, void *)
{
	return size * nmemb;
}

UploadConfig::UploadConfig(CURL *curlEnv, const std::vector<std::uint8_t> &dataSource) :
	m_curlEnv{curlEnv},
	m_readBytesCounter{dataSource}
{
	curl_easy_setopt(m_curlEnv, CURLOPT_READDATA, m_readBytesCounter);
	curl_easy_setopt(m_curlEnv, CURLOPT_READFUNCTION, UploadVectorDataCallback);
}
UploadConfig::~UploadConfig()
{
	curl_easy_setopt(m_curlEnv, CURLOPT_READDATA, nullptr);
	curl_easy_setopt(m_curlEnv, CURLOPT_READFUNCTION, DoNothingWithUploads);
}
size_t UploadConfig::UploadVectorDataCallback(void *buffer, size_t size, size_t nitems, void *bytesCounterPtr)
{
	auto &bytesCounter = *static_cast<CURL_UTILS::ReadBytesCounter *>(bytesCounterPtr);
	size_t bytesSent = bytesCounter.bytesRead;

	std::uint8_t *bytesBuffer{reinterpret_cast<std::uint8_t *>(buffer)};
	while(bytesSent < bytesCounter.m_bytes.size() &&
		bytesSent < size * nitems)
	{
		bytesBuffer[bytesSent - bytesCounter.bytesRead] =
			bytesCounter.m_bytes[bytesSent];
		++bytesSent;
	}

	return bytesSent - bytesCounter.bytesRead;
}

UseHeadersConfig::UseHeadersConfig(CURL *curlEnv, const Headers &headersToUse) :
	m_curlEnv{curlEnv},
	m_curlHeaders{HeadersToCurlSList(headersToUse)}
{
	curl_easy_setopt(m_curlEnv, CURLOPT_HTTPHEADER, m_curlHeaders.get());
}
void UseHeadersConfig::SetHeaders(const Headers &headersToUse)
{
	auto newCurlHeaders{HeadersToCurlSList(headersToUse)};
	m_curlHeaders.swap(newCurlHeaders);
	curl_easy_setopt(m_curlEnv, CURLOPT_HTTPHEADER, m_curlHeaders.get());
}

size_t DoNothingWithUploads(void *, size_t, size_t, void *)
{
	return 0;
}

PCURLList HeadersToCurlSList(const Headers &headers)
{
	struct curl_slist *headers_list{NULL};
	for(auto const &[key, value] : headers)
	{
		headers_list = curl_slist_append(headers_list, (key + ": " + value).c_str());
	}
	return PCURLList{headers_list};
}

bool ProcessCode(CURLcode code) noexcept
{
	return code == CURLE_OK;
}

void TrimString(std::string &stringToTrim)
{
	const auto startOfString = std::find_if_not(std::begin(stringToTrim), std::end(stringToTrim), isspace);
	const auto endOfString = std::find_if_not(std::rbegin(stringToTrim), std::rend(stringToTrim), isspace);

	std::copy(startOfString, (endOfString + 1).base(), std::begin(stringToTrim));
}

}
