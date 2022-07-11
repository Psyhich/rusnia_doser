#ifndef API_INTERFACE_H
#define API_INTERFACE_H

#include <optional>
#include <string>
#include <vector>
#include <mutex>

#include "nlohmann/json.hpp"

#include "curl_wrapper.h"
#include "target.hpp"
#include "multithread.h"

class Informator
{
public:
	Informator() = default;

	inline static void ContactSources(const TaskController &task)
	{
		SPDLOG_INFO("Loading all available API hosts");
		std::call_once(m_loadResourcesFlag, LoadResouces, std::cref(task));
	}

	bool LoadNewData() noexcept;

	inline std::optional<std::vector<Proxy>> GetProxies() const noexcept
	{
		return m_proxies;
	}

	inline std::optional<CURI> GetTarget() const noexcept
	{
		return m_target;
	}

	inline std::optional<Attackers::AttackMethod> GetMethod() const noexcept
	{
		return m_method;
	}

private:
	static void LoadResouces(const TaskController &task) noexcept;

	void ParseProxies(const nlohmann::json &jsonObject);
	void ParseTarget(const nlohmann::json &jsonObject);
	void ParseMethod(const nlohmann::json &jsonObject);

private:
	inline static std::vector<std::string> m_availableResources;
	inline static std::once_flag m_loadResourcesFlag;

	std::optional<std::vector<Proxy>> m_proxies;
	std::optional<CURI> m_target;
	std::optional<Attackers::AttackMethod> m_method;
};

#endif // API_INTERFACE_H
