#include "curl_wrapper.h"

CURLLoader::CURLLoader() noexcept
{
	std::call_once(g_curlInitFlag, curl_global_init, CURL_GLOBAL_ALL);

	m_curlEnv.reset(curl_easy_init());
	if(!m_curlEnv)
	{
		std::cerr << "Failed to create curl instance" << std::endl;
		return;
	}

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_HTTPPROXYTUNNEL, true);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_SSL_VERIFYHOST, 0L);
}

void CURLLoader::SetTarget(const std::string &address) noexcept
{
	if(!m_curlEnv)
	{
		return;
	}
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_URL, address.c_str());

	if(size_t pos = address.find("://");
		pos != std::string::npos)
	{
		std::string scheme{address.begin(), address.begin() + pos};
		curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXYTYPE, scheme.c_str());
	}
}

void CURLLoader::SetProxy(const std::string &proxyIP, const std::string &proxyAuth) noexcept
{
	if(!m_curlEnv)
	{
		return;
	}
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_PROXY, proxyIP.c_str());
	if(!proxyAuth.empty())
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

void CURLLoader::SetHeaders(const Headers &headers) noexcept
{
	struct curl_slist *headers_list{NULL};
	for(auto const &[key, value] : headers)
	{
		headers_list = curl_slist_append(headers_list, (key + value).c_str());
	}

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_HTTPHEADER, headers_list);
}

std::optional<Response> CURLLoader::Download(long timeout) noexcept
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

std::optional<long> CURLLoader::Ping(long timeout) noexcept
{
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(m_curlEnv.get(), CURLOPT_WRITEFUNCTION, DoNothingCallback);

	curl_easy_setopt(m_curlEnv.get(), CURLOPT_TIMEOUT, timeout);

	CURLcode code = curl_easy_perform(m_curlEnv.get());
	if(!ProcessCode(code))
	{
		return {};
	}
	
	long httpCode{0};
	curl_easy_getinfo(m_curlEnv.get(), CURLINFO_RESPONSE_CODE, &httpCode);
	return httpCode;
}

size_t CURLLoader::DataCallback(void *contents, size_t size, size_t nmemb, void *userp) noexcept
{
	std::vector<char> &readBuffer = *static_cast<std::vector<char>*>(userp);

	readBuffer.reserve(readBuffer.size() + nmemb);

	for(size_t i = 0; i < nmemb; i++)
	{
		readBuffer.push_back(((char*)contents)[i]);
	}

	return nmemb;
}

