#include <algorithm>
#include <cctype>

#include <spdlog/spdlog.h>

#include "http_wrapper.h"

HTTPWrapper::HTTPWrapper() noexcept
{
	std::call_once(g_curlInitFlag, curl_global_init, CURL_GLOBAL_ALL);

	m_curlEnv.reset(curl_easy_init());
	if(!m_curlEnv)
	{
		// TODO: Change this function to handle
		// this error not just return from func
		SPDLOG_CRITICAL("Failed to create curl instance\n");
		return;
	}

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_HTTPPROXYTUNNEL, true);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_SSL_VERIFYHOST, 0L);
}

void HTTPWrapper::SetTarget(const std::string &address) noexcept
{
	if(!m_curlEnv)
	{
		return;
	}
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_URL, address.c_str());
}

void HTTPWrapper::SetProxy(const std::string &proxyIP, const std::string &proxyAuth) noexcept
{
	if(!m_curlEnv)
	{
		return;
	}
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXY, proxyIP.c_str());
	if(!proxyAuth.empty() || proxyAuth.size() >= 1)
	{
		curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXYUSERPWD, proxyAuth.c_str());
	}

	char *url{nullptr};

	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_EFFECTIVE_URL, &url);

	if(url)
	{
		std::string data{std::move(url)};
		if(size_t pos = data.find("://");
			pos != std::string::npos)
		{
			std::string scheme{data.begin(), data.begin() + pos};
			curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXYTYPE, scheme.c_str());
		}
	}
}

void HTTPWrapper::SetHeaders(const Headers &headers) noexcept
{
	struct curl_slist *headers_list{NULL};
	for(auto const &[key, value] : headers)
	{
		headers_list = curl_slist_append(headers_list, (key + ": " + value).c_str());
	}

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_HTTPHEADER, headers_list);
}

std::optional<Response> HTTPWrapper::Download(long timeout) noexcept
{
	std::vector<char> readBuffer;

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_WRITEDATA, &readBuffer);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_WRITEFUNCTION, DataCallback);
	
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_CONNECTTIMEOUT, timeout);

	CURLcode code = curl_easy_perform(m_curlEnv.get());

	if(!ProcessCode(code))
	{
		return {};
	}
	
	long httpCode{0};
	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_RESPONSE_CODE, &httpCode);

	return Response(httpCode, std::move(readBuffer));
}

std::optional<long> HTTPWrapper::Ping(long timeout, Headers* headers) noexcept
{
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_TIMEOUT, timeout);

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_WRITEFUNCTION, DoNothingCallback);

	if(headers != nullptr)
	{
		curl_easy_setopt(m_curlEnv.get(), CURLOPT_HEADERFUNCTION, ProcessHeaderCallback);
		curl_easy_setopt(m_curlEnv.get(), CURLOPT_HEADERDATA, headers);
	}

	CURLcode code = curl_easy_perform(m_curlEnv.get());

	if(headers != nullptr)
	{
		curl_easy_setopt(m_curlEnv.get(), CURLOPT_HEADERFUNCTION, DoNothingWithHeader);
		curl_easy_setopt(m_curlEnv.get(), CURLOPT_HEADERDATA, NULL);
	}

	if(!ProcessCode(code))
	{
		return {};
	}
	
	long httpCode{0};
	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_RESPONSE_CODE, &httpCode);
	return httpCode;
}

size_t HTTPWrapper::DataCallback(void *contents, size_t size, size_t nmemb, void *userp) noexcept
{
	std::vector<char> &readBuffer = *static_cast<std::vector<char>*>(userp);

	readBuffer.reserve(readBuffer.size() + nmemb);

	for(size_t i = 0; i < nmemb; i++)
	{
		readBuffer.push_back(((char*)contents)[i]);
	}

	return nmemb * size;
}

size_t HTTPWrapper::ProcessHeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
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

	Headers &headers = *static_cast<Headers *>(userdata);
	headers.emplace(std::move(key), std::move(value));

	return size * nitems;
}

void HTTPWrapper::TrimString(std::string &stringToTrim)
{
	const auto startOfString = std::find_if_not(std::begin(stringToTrim), std::end(stringToTrim), isspace);
	const auto endOfString = std::find_if_not(std::rbegin(stringToTrim), std::rend(stringToTrim), isspace);

	std::copy(startOfString, (endOfString + 1).base(), std::begin(stringToTrim));
}
