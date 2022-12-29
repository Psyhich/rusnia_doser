#ifndef MY_URI_H
#define MY_URI_H

#include <string>
#include <filesystem>
#include <optional>
#include <algorithm>
#include <array>

#include <spdlog/fmt/ostr.h>

/// Utility class for easier parsing of URIs 
/// specified by RFC3986
class URI
{
public:
	URI() = default;
	URI(const std::string &cStringToSet, bool strictParse=false);

	inline std::string GetFullURI() const 
	{
		return ConstructFrom(m_protocol, m_host,
			m_port, m_path, m_query, m_fragment);
	}
	std::string GetProtocol() const noexcept;
	void SetProtocol(const std::string &protocol);

	std::optional<int> GetPort() const noexcept;
	void SetPort(std::optional<int> port);

	std::string GetPureAddress() const noexcept;
	void SetPureAddress(const std::string &pureAddress);

	std::filesystem::path GetPath() const noexcept;
	void SetPath(const std::filesystem::path &path);

	std::string GetQuery() const noexcept;
	void SetQuery(const std::string &query);

	std::string GetFragment() const noexcept;
	void SetFragment(const std::string &fragment);

	friend inline std::ostream &operator<<(std::ostream &os, const URI &uri) noexcept
	{
		return os << uri.GetFullURI();
	}

	friend bool operator<(const URI& cLURIToCompare, const URI &cRURIToCompare) noexcept;
	friend bool operator==(const URI& cLURIToCompare, const URI &cRURIToCompare) noexcept;
private:
	static inline bool CanBeUsedInProtocol(char chCharToCheck) noexcept
	{
		return (chCharToCheck >= 'A' && chCharToCheck <= 'Z') || 
			(chCharToCheck >= 'a' && chCharToCheck <= 'z') ||
			chCharToCheck == '.' || chCharToCheck == '+' ||
			chCharToCheck == '-';
	}

	static std::string ConstructFrom(const std::string &protocol, 
		const std::string &address, const std::optional<int> &port, 
		const std::filesystem::path &path, const std::string &query, 
		const std::string &fragment);

	using StringIter = std::string::const_iterator;
	void ParseProtocol(StringIter &parseStart, StringIter parseEnd, bool strictParse);
	void ParseAddress(StringIter &parseStart, StringIter parseEnd, bool strictParse);
	void ParsePort(StringIter &parseStart, StringIter parseEnd, bool strictParse);
	void ParsePath(StringIter &parseStart, StringIter parseEnd, bool strictParse);
	void ParseQuery(StringIter &parseStart, StringIter parseEnd, bool strictParse);
	void ParseFragment(StringIter &parseStart, StringIter parseEnd, bool strictParse);

	static const constexpr std::array POSSIBLE_ADDRESS_END{':', '#', '?', '/', '\0'};
	static const constexpr std::array POSSIBLE_PORT_END{'#', '?', '/', '\0'};
	static const constexpr std::array POSSIBLE_PATH_END{'#', '?', '\0'};

private:
	inline static const std::string PROTOCOL_SEPARATOR{"://"};

	std::string m_protocol;
	std::string m_host;
	std::optional<std::uint16_t> m_port;
	std::filesystem::path m_path;
	std::string m_query;
	std::string m_fragment;
};

template <>
struct fmt::formatter<URI>
{
	constexpr auto parse(format_parse_context& ctx) ->
		decltype(ctx.begin())
	{
		return ctx.end();
	}

	template <typename FormatContext>
	auto format(const URI& uri, FormatContext& ctx) const ->
		decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "{}", uri.GetFullURI());
	}
};

#endif // MY_URI_H
