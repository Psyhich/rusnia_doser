#include <algorithm>
#include <exception>
#include <stdexcept>

#include "spdlog/fmt/bundled/format.h"

#include "uri.h"

const std::string CURI::sProtocolSeparator{"://"};

std::optional<std::string> CURI::ConstructFrom(const std::optional<std::string> &protocol, 
	const std::string &address, const std::optional<int> &port, 
	const std::optional<std::filesystem::path> &path, const std::optional<std::string> &query, 
	const std::optional<std::string> &fragment)
{
	if(address.size() == 0 && (!protocol || std::all_of(std::begin(*protocol), 
		std::end(*protocol), CanBeUsedInProtocol)))
	{
		return (protocol ? *protocol + "://" : "") + address +
			(port ? ':' + std::to_string(*port) : "") +
			(path ? path->string() : "") +
			(query ? '?' + *query : "") +
			(fragment ? '#' + *fragment : "");
	}
	return {};
}

CURI::CURI(const std::string& cStringToSet) : m_originalString{cStringToSet}
{
}

std::optional<std::string> CURI::GetProtocol() const noexcept
{
	const std::size_t cnProtocolEnd = m_originalString.find(sProtocolSeparator);

	if(cnProtocolEnd == std::string::npos ||
		!std::all_of(m_originalString.begin(), m_originalString.begin() + cnProtocolEnd, 
		CanBeUsedInProtocol))
	{
		return std::nullopt;
	}

	return m_originalString.substr(0, cnProtocolEnd);
}

bool CURI::SetProtocol(const std::string &protocol) noexcept
{
	auto newURI = ConstructFrom(protocol, *GetPureAddress(), 
			GetPort(), GetPath(), GetQuery(), GetFragment());
	if(newURI)
	{
		m_originalString = std::move(*newURI);
	}
	return newURI.has_value();
}

std::optional<std::string> CURI::GetPureAddress() const noexcept
{
	// We should take in mind protocol and port, ideally address can end with / ? #
	// Checking for protocol start
	auto addressStart = m_originalString.begin(); 
	const std::size_t protocolStart{m_originalString.find(sProtocolSeparator)};
	if(protocolStart != std::string::npos)
	{
		std::advance(addressStart, protocolStart + sProtocolSeparator.size());
	}

	// Checking for address end
	auto addressEnd = std::find_first_of(addressStart, m_originalString.end(), 
		POSSIBLE_ADDRESS_END.begin(), POSSIBLE_ADDRESS_END.end());

	return std::string{addressStart, addressEnd};
}
bool CURI::SetPureAddress(const std::string &address) noexcept
{
	auto newURI = ConstructFrom(GetProtocol(), address, GetPort(), 
		GetPath(), GetQuery(), GetFragment());
	if(newURI)
	{
		m_originalString = std::move(*newURI);
	}
	return newURI.has_value();
}

std::optional<int> CURI::GetPort() const noexcept
{
	// Port specified between ':' and '#' '?' '/' chars
	// Also we should omit protocol specifier ://
	auto portStartPos = m_originalString.begin();
	// Omiting ://
	if(const std::size_t cnProtocolStart = m_originalString.find(sProtocolSeparator); 
		cnProtocolStart != std::string::npos)
	{
		std::advance(portStartPos, cnProtocolStart + sProtocolSeparator.size());
	}

	// Looking for port start
	portStartPos = std::find(portStartPos, m_originalString.end(), ':');

	// If we didn't find port setting it to nullopt
	if(portStartPos == m_originalString.end())
	{
		return std::nullopt;
	}
	// If we find port, skip ':'
	++portStartPos;

	// Now looking for port end
	auto portEndPos = std::find_first_of(portStartPos, m_originalString.end(), 
		POSSIBLE_PORT_END.begin(), POSSIBLE_PORT_END.end());

	const std::string csStringPort{portStartPos, portEndPos};
	// Checking if given port is valid
	try {
		std::size_t nLastParsedCharIndex{0};
		const int ciParsedNumber = std::stoi(csStringPort.c_str(), &nLastParsedCharIndex);
	
		// This is largest port number
		// For better safety checking if last parsed char is last in string, if not we have string like: "1234asd"
		if(ciParsedNumber > 65535 || 
			nLastParsedCharIndex != csStringPort.size()) 
		{
			return std::nullopt;
		}
		return ciParsedNumber;
	}
	// std::stoi can throw invalid_argument or out_of_range, 
	// So catching any exceptions
	catch(...)
	{
		return std::nullopt;
	}
}
bool CURI::SetPort(int port) noexcept
{
	auto newURI = ConstructFrom(GetProtocol(), *GetPureAddress(), 
		port, GetPath(), GetQuery(), GetFragment());
	if(newURI)
	{
		m_originalString = std::move(*newURI);
	}
	return newURI.has_value();
}


std::optional<std::filesystem::path> CURI::GetPath() const noexcept
{
	auto pathStartPos = m_originalString.begin();
	// Omiting ://
	if(const std::size_t cnProtocolStart = m_originalString.find(sProtocolSeparator); 
		cnProtocolStart != std::string::npos)
	{
		std::advance(pathStartPos, cnProtocolStart + sProtocolSeparator.size());
	}

	// Checking for path start
	pathStartPos = std::find(pathStartPos, m_originalString.end(), '/');
	if(pathStartPos == m_originalString.end())
	{
		return std::nullopt;
	}
	
	auto pathEndPos = std::find_first_of(pathStartPos, m_originalString.end(), 
		POSSIBLE_PATH_END.begin(), POSSIBLE_PATH_END.end());

	// taking in mind zero path(proto://some.page.com/)
	if(std::distance(pathStartPos, pathEndPos) <= 1)
	{
		return std::nullopt;
	}

	return std::filesystem::path(std::string{pathStartPos, pathEndPos});
}
bool CURI::SetPath(const std::filesystem::path &path) noexcept
{
	auto newURI = ConstructFrom(GetProtocol(), *GetPureAddress(), GetPort(), 
		path, GetQuery(), GetFragment());
	if(newURI)
	{
		m_originalString = std::move(*newURI);
	}
	return newURI.has_value();
}

std::optional<std::string> CURI::GetQuery() const noexcept
{
	auto queryStart = std::find(m_originalString.begin(), m_originalString.end(), '?');

	if(queryStart == m_originalString.end())
	{
		return std::nullopt;
	}
	//
	// Skipping '?'
	++queryStart;

	auto queryEnd = std::find(queryStart, m_originalString.end(), '#');

	// Checking if it's not empty
	if(std::distance(queryStart, queryEnd) == 0)
	{
		return std::nullopt;
	}

	return std::string{queryStart, queryEnd};
}
bool CURI::SetQuery(const std::string &query) noexcept
{
	auto newURI = ConstructFrom(GetProtocol(), *GetPureAddress(), GetPort(), 
		GetPath(), query, GetFragment());
	if(newURI)
	{
		m_originalString = std::move(*newURI);
	}
	return newURI.has_value();
}

std::optional<std::string> CURI::GetFragment() const noexcept
{
	auto fragmentStart = std::find(m_originalString.begin(), m_originalString.end(), '#');

	if(fragmentStart == m_originalString.end())
	{
		return std::nullopt;
	}
	// Skipping '#'
	++fragmentStart;

	if(std::distance(fragmentStart, m_originalString.end()) == 0)
	{
		return std::nullopt;
	}
	return std::string{fragmentStart, m_originalString.end()};
}
bool CURI::SetFragment(const std::string &fragment) noexcept
{
	auto newURI = ConstructFrom(GetProtocol(), *GetPureAddress(), GetPort(), 
		GetPath(), GetQuery(), fragment);
	if(newURI)
	{
		m_originalString = std::move(*newURI);
	}
	return newURI.has_value();
}

bool operator<(const CURI& cLURIToCompare, const CURI &cRURIToCompare) noexcept
{
	return cLURIToCompare.m_originalString < cRURIToCompare.m_originalString;
}
bool operator==(const CURI& cLURIToCompare, const CURI &cRURIToCompare) noexcept
{
	return cLURIToCompare.m_originalString == cRURIToCompare.m_originalString;
}
