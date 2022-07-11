#ifndef MY_URI_H
#define MY_URI_H

#include <string>
#include <filesystem>
#include <optional>
#include <algorithm>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

/// Utility class for easier parsing of URIs 
/// specified by RFC3986
class CURI
{
public:
	CURI() = default;
	CURI(const std::string& cStringToSet);

	inline std::string GetFullURI() const noexcept 
	{
		return m_originalString;
	}
	std::optional<std::string> GetProtocol() const noexcept;
	bool SetProtocol(const std::string &protocol) noexcept;

	std::optional<int> GetPort() const noexcept;
	bool SetPort(int port) noexcept;

	std::optional<std::string> GetPureAddress() const noexcept;
	bool SetPureAddress(const std::string &pureAddress) noexcept;

	std::optional<std::filesystem::path> GetPath() const noexcept;
	bool SetPath(const std::filesystem::path &path) noexcept;

	std::optional<std::string> GetQuery() const noexcept;
	bool SetQuery(const std::string &query) noexcept;

	std::optional<std::string> GetFragment() const noexcept;
	bool SetFragment(const std::string &fragment) noexcept;

	friend inline std::ostream &operator<<(std::ostream &os, const CURI &uri) noexcept
	{
		return os << uri.GetFullURI();
	}

	friend bool operator<(const CURI& cLURIToCompare, const CURI &cRURIToCompare) noexcept;
	friend bool operator==(const CURI& cLURIToCompare, const CURI &cRURIToCompare) noexcept;
private:
	static inline bool CanBeUsedInProtocol(char chCharToCheck) noexcept
	{
		return (chCharToCheck >= 'A' && chCharToCheck <= 'Z') || 
			(chCharToCheck >= 'a' && chCharToCheck <= 'z') ||
			chCharToCheck == '.' || chCharToCheck == '+' ||
			chCharToCheck == '-';
	}

	static std::optional<std::string> ConstructFrom(const std::optional<std::string> &protocol, 
		const std::string &address, const std::optional<int> &port, 
		const std::optional<std::filesystem::path> &path, const std::optional<std::string> &query, 
		const std::optional<std::string> &fragment);
private:
	static const constexpr std::array<char, 4> POSSIBLE_ADDRESS_END{':', '#', '?', '/'};
	static const constexpr std::array<char, 3> POSSIBLE_PORT_END{'#', '?', '/'};
	static const constexpr std::array<char, 2> POSSIBLE_PATH_END{'#', '?'};
	static const std::string sProtocolSeparator;
	std::string m_originalString;
};

template<typename OStream>
OStream &operator<<(OStream &os, const CURI& uri)
{
	return fmt::format_to(std::ostream_iterator<char>(os), "{}", uri.GetFullURI());
}

#endif // MY_URI_H
