#include "proxy_manager.h"
#include "config.h"

#include "curl_wrapper.h"
#include "utils.h"

ProxyManager::ProxyRating ProxyManager::FindProxy(const Proxy &proxyToFind)
{
	std::lock_guard<std::mutex> ratingLock{m_ratingMutex};

	auto foundVal = m_proxyRating.find(proxyToFind);
	if(foundVal == m_proxyRating.end())
	{
		m_proxyRating[proxyToFind] = STARTING_RATING;
		return {proxyToFind, STARTING_RATING};
	}

	return *foundVal;
}

// bool ProxyManager::CheckProxy(const Proxy &proxyToCheck)
// {
// 	FindProxy(proxyToCheck);
//
// 	CURLLoader checker;
// 	checker.SetProxy(proxyToCheck.first, proxyToCheck.second);
// 	
// 	checker.SetHeaders({
// 		{"Content-Type", "*/*"},
// 		{"Cf-Visitor", "https"},
// 		{"User-Agent", ChoseUseragent()},
// 		{"Connection", "keep-alive"},
// 		{"Accept", "application/json, text/plain, */*"},
// 		{"Accept-Language", "ru"},
// 		{"Accept-Encoding", "gzip, deflate, br"},
// 		{"X-Forwarded-Proto", "https"},
// 		{"X-Forwarder-For", proxyToCheck.first},
// 		{"Cache-Control", "no-store"}
// 	});
//
// 	bool succesfulyConnected{false};
// 	
// 	for(int i = 0; i < ProxyConfig::PROXY_TRIES; i++)
// 	{
// 		const long code = checker.Ping();
// 		if(code >= 200 && code < 400)
// 		{
// 			succesfulyConnected = true;
// 			break;
// 		}
// 		else
// 		{
// 			LowerRating(proxyToCheck);
// 		}
// 	}
//
// 	return succesfulyConnected;
// }

bool ProxyManager::LowerRating(const Proxy &proxyToLower)
{
	std::lock_guard<std::mutex> proxyLock{m_ratingMutex};
	return ((m_proxyRating[proxyToLower] -= 1) > 0);
}
