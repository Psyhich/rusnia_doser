#include <cstdint>
#include <cassert>

#include <algorithm>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

#include "uri.h"

std::string URI::ConstructFrom(const std::string &protocol, 
	const std::string &address, const std::optional<int> &port, 
	const std::filesystem::path &path, const std::string &query, 
	const std::string &fragment)
{
	std::string constructedURI;
	std::size_t URISizeToReserve{address.size()};

	const bool protocolNotEmpty = !protocol.empty();
	if(protocolNotEmpty)
	{
		URISizeToReserve += protocol.size() + 3;
	}

	std::optional<std::string> stringifiedPort{std::nullopt};
	if(port.has_value())
	{
		stringifiedPort = std::to_string(*port);
		URISizeToReserve += stringifiedPort->size() + 1;
	}

	const bool queryNotEmpty = !query.empty();
	if(queryNotEmpty)
	{
		URISizeToReserve += protocol.size() + 1;
	}

	const bool pathNotEmpty = !path.empty();
	if(pathNotEmpty)
	{
		URISizeToReserve += path.string().size() + (path.string()[0] == '/');
	}

	const bool fragmentNotEmpty = !fragment.empty();
	if(fragmentNotEmpty)
	{
		URISizeToReserve += fragment.size() + 1;
	}
	constructedURI.reserve(URISizeToReserve);

	constructedURI.append(protocol);
	if(protocolNotEmpty)
	{
		constructedURI.append(PROTOCOL_SEPARATOR);
	}
	constructedURI.append(address);
	
	if(stringifiedPort)
	{
		constructedURI.push_back(':');
		constructedURI.append(*stringifiedPort);
	}

	if(pathNotEmpty && path.string()[0] != '/')
	{
		constructedURI.push_back('/');
	}
	constructedURI.append(path.string());

	if(queryNotEmpty)
	{
		constructedURI.push_back('?');
		constructedURI.append(query);
	}
	if(fragmentNotEmpty)
	{
		constructedURI.push_back('#');
		constructedURI.append(fragment);
	}

	return constructedURI;
}

void URI::ParseProtocol(StringIter &parseStart, StringIter parseEnd, bool strictParse)
{
	const auto protocolEnd = std::search(parseStart, parseEnd,
		begin(PROTOCOL_SEPARATOR), end(PROTOCOL_SEPARATOR));
	if(protocolEnd == parseEnd)
	{
		if(strictParse)
		{
			throw std::runtime_error("Strict parsing requires protocol");
		}
		return;
	}
	if(!std::all_of(parseStart, protocolEnd, CanBeUsedInProtocol))
	{
		throw std::runtime_error(fmt::format("Protocol {} string uses forbidden characters",
			std::string{parseStart, protocolEnd}));
	}
	else
	{
		m_protocol = std::string{parseStart, protocolEnd};
		if(m_protocol.empty() && strictParse)
		{
			throw std::runtime_error("Protocol is not allowed being empty in strict parsing");
		}
		parseStart = protocolEnd;
		std::advance(parseStart, PROTOCOL_SEPARATOR.size());
	}
}

void URI::ParseAddress(StringIter &parseStart, StringIter parseEnd, bool strictParse)
{
	const auto hostEnd = std::find_first_of(parseStart, parseEnd, 
		begin(POSSIBLE_ADDRESS_END), end(POSSIBLE_ADDRESS_END));
	m_host = std::string{parseStart, hostEnd};

	if(m_host.empty() && strictParse)
	{
		throw std::runtime_error("Host address cannot be empty in strict parsing");
	}

	parseStart = hostEnd;
}
void URI::ParsePort(StringIter &parseStart, StringIter parseEnd, bool strictParse)
{
	if(*parseStart == ':')
	{
		std::advance(parseStart, 1);
		const auto portEnd = std::find_first_of(parseStart, parseEnd,
			begin(POSSIBLE_PORT_END), end(POSSIBLE_PORT_END));

		if(strictParse &&
			std::distance(parseStart, portEnd) == 0)
		{
			throw std::runtime_error("Port colon char exists but no port specified in strict parsing");
		}
		const std::string portString{parseStart, portEnd};
		const int parsedPort = std::stoi(portString);
		if(parsedPort > std::numeric_limits<std::uint16_t>::max())
		{
			throw std::runtime_error("Port exceeds max allowed port number in strict parsing");
		}
		m_port = parsedPort;

		std::advance(parseStart, portString.size());
	}
}

void URI::ParsePath(StringIter &parseStart, StringIter parseEnd, bool)
{
	if(*parseStart == '/')
	{
		const auto pathStart = parseStart;
		std::advance(parseStart, 1);
		const auto pathEnd = std::find_first_of(parseStart, parseEnd,
			begin(POSSIBLE_PATH_END), end(POSSIBLE_PATH_END));

		m_path = std::string{pathStart, pathEnd};
		parseStart = pathEnd;
	}
}

void URI::ParseQuery(StringIter &parseStart, StringIter parseEnd, bool strictParse)
{
	if(*parseStart == '?')
	{
		std::advance(parseStart, 1);

		const auto queryEnd = std::find(parseStart, parseEnd, '#');
		if(strictParse &&
			std::distance(parseStart, queryEnd) == 0)
		{
			throw std::runtime_error("Query character specified but no query provided in strict parsing");
		}
		m_query = std::string{parseStart, queryEnd};
		parseStart = queryEnd;
	}
}

void URI::ParseFragment(StringIter &parseStart, StringIter parseEnd, bool strictParse)
{
	if(*parseStart == '#')
	{
		std::advance(parseStart, 1);

		if(strictParse &&
			std::distance(parseStart, parseEnd))
		{
			throw std::runtime_error("Fragment character specified but no fragment provided in strict parsing");
		}
		m_fragment = std::string{parseStart, parseEnd};
		parseStart = parseEnd;
	}
}

URI::URI(const std::string& stringToSet, bool strictParse)
{
	auto parsingStart = begin(stringToSet);
	const auto stringEnd = end(stringToSet);

	ParseProtocol(parsingStart, stringEnd, strictParse);
	ParseAddress(parsingStart, stringEnd, strictParse);
	ParsePort(parsingStart, stringEnd, strictParse);
	ParsePath(parsingStart, stringEnd, strictParse);
	ParseQuery(parsingStart, stringEnd, strictParse);
	ParseFragment(parsingStart, stringEnd, strictParse);
}

std::string URI::GetProtocol() const noexcept
{
	return m_protocol;
}

void URI::SetProtocol(const std::string &protocol)
{
	m_protocol = protocol;

}

std::string URI::GetPureAddress() const noexcept
{
	return m_host;
}
void URI::SetPureAddress(const std::string &address)
{
	m_host = address;
}

std::optional<int> URI::GetPort() const noexcept
{
	return m_port;
}
void URI::SetPort(std::optional<int> port)
{
	m_port = port;
}


std::filesystem::path URI::GetPath() const noexcept
{
	return m_path;
}
void URI::SetPath(const std::filesystem::path &path)
{
	m_path = path;
}

std::string URI::GetQuery() const noexcept
{
	return m_query;
}
void URI::SetQuery(const std::string &query)
{
	m_query = query;
}

std::string URI::GetFragment() const noexcept
{
	return m_fragment;
}
void URI::SetFragment(const std::string &fragment)
{
	m_fragment = fragment;
}

bool operator<(const URI& cLURIToCompare, const URI &cRURIToCompare) noexcept
{
	return cLURIToCompare.GetFullURI() < cRURIToCompare.GetFullURI();
}
bool operator==(const URI& cLURIToCompare, const URI &cRURIToCompare) noexcept
{
	return cLURIToCompare.GetFullURI() == cRURIToCompare.GetFullURI();
}
