#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstdint>
#include <string>
#include <map>
#include <vector>

namespace HTTP
{

using Headers = std::map<std::string, std::string>;
using Proxy = std::pair<std::string, std::string>;

inline static const Headers BASE_HEADERS
	{
		{"Content-Type", "*/*"},
		{"Connection", "keep-alive"},
		{"Accept", "application/json, text/plain, */*"},
		{"Accept-Language", "ru"},
		{"Accept-Encoding", "gzip, deflate, br"},
		{"Cache-Control", "no-store"}
	};

using HTTPCode = std::uint16_t;

struct Response
{
public:
	Response() = default;
	Response(HTTPCode code, std::vector<char> &&data) :
		m_code{code},
		m_data{std::move(data)}
	{
	}

	HTTPCode m_code;
	std::vector<char> m_data;
};

struct Payload
{
	Headers m_headers;
	std::vector<std::uint8_t> data;
};

}

#endif // RESPONSE_HPP
