#include <algorithm>
#include <cctype>
#include <stdexcept>

#include <curl/curl.h>
#include <spdlog/spdlog.h>

#include "curl_utils.h"
#include "http_wrapper.h"

namespace HTTP
{

HTTPWrapper::HTTPWrapper() :
	m_headersConfig(nullptr, {})
{
	m_curlEnv.reset(curl_easy_init());
	if(!m_curlEnv)
	{
		// TODO: Change this function to handle
		// this error not just return from func
		SPDLOG_CRITICAL("Failed to create curl instance\n");
		return;
	}

	SetDefaultOptions();
}

void HTTPWrapper::SetTarget(const std::string &address)
{
	if(!m_curlEnv)
	{
		return;
	}
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_URL, address.c_str());
}

// TODO: save headers internaly and only leave
// external interface to add/remove custom headers
void HTTPWrapper::SetProxy(const Proxy &proxy)
{
	if(!m_curlEnv)
	{
		return;
	}
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXY, proxy.first.c_str());
	if(!proxy.second.empty() || proxy.second.size() >= 1)
	{
		curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXYUSERPWD, proxy.second.c_str());
	}

	const char *url{nullptr};
	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_EFFECTIVE_URL, &url);

	if(url)
	{
		std::string data{url};
		if(size_t pos = data.find("://");
			pos != std::string::npos)
		{
			std::string scheme{data.begin(), data.begin() + pos};
			curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXYTYPE, scheme.c_str());
		}
	}
}

void HTTPWrapper::SetHeaders(const Headers &headers)
{
	m_headersConfig.SetHeaders(headers);
}

std::optional<Response> HTTPWrapper::Download(long timeout)
{
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_CONNECTTIMEOUT, timeout);

	std::vector<char> readBuffer;
	CURL_UTILS::DownloadConfig downloadConfig{m_curlEnv.get(), readBuffer};

	const CURLcode code = curl_easy_perform(m_curlEnv.get());
	if(!CURL_UTILS::ProcessCode(code))
	{
		return {};
	}
	
	long httpCode{0};
	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_RESPONSE_CODE, &httpCode);

	return Response{static_cast<HTTPCode>(httpCode), std::move(readBuffer)};
}

std::optional<HTTPCode> HTTPWrapper::Ping(long timeout, Headers* headers)
{
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_TIMEOUT, timeout);

	std::optional<CURL_UTILS::SaveHeadersConfig> saveHeadersConfig{std::nullopt};
	if(headers != nullptr)
	{
		saveHeadersConfig.emplace(m_curlEnv.get(), *headers);
	}

	const CURLcode code = curl_easy_perform(m_curlEnv.get());
	if(!CURL_UTILS::ProcessCode(code))
	{
		return {};
	}
	
	long httpCode{0};
	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_RESPONSE_CODE, &httpCode);
	return httpCode;
}

std::optional<Response> HTTPWrapper::Send(const Payload &payload, long timeout)
{
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_CONNECTTIMEOUT, timeout);

	Response response;
	CURL_UTILS::DownloadConfig downloadConfig{m_curlEnv.get(), response.m_data};
	CURL_UTILS::UploadConfig uploadConfig{m_curlEnv.get(), payload.data};

	SetHeaders(payload.m_headers);

	const CURLcode code = curl_easy_perform(m_curlEnv.get());
	if(!CURL_UTILS::ProcessCode(code))
	{
		return {};
	}
	
	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_RESPONSE_CODE, &response.m_code);
	return response;
}

void HTTPWrapper::SetDefaultOptions()
{
	// TODO: think about changing CURLOPT_FOLLOWLOCATION
	// to false for omiting redirecting to 0.0.0.0
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_HTTPPROXYTUNNEL, true);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_SSL_VERIFYHOST, false);

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_READFUNCTION,
		CURL_UTILS::DoNothingWithUploads);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_WRITEFUNCTION,
		CURL_UTILS::DoNothingWithDownloads);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_HEADERFUNCTION,
		CURL_UTILS::DoNothingWithHeader);

	m_headersConfig = CURL_UTILS::UseHeadersConfig{m_curlEnv.get(), {}};
}

}
