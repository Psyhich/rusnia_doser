#ifndef API_INTERFACE_H
#define API_INTERFACE_H

#include <optional>
#include <string>
#include <vector>
#include <mutex>

#include "nlohmann/json.hpp"

class Informator
{
public:
	enum class AttackMethod
	{
		HTTPAttack,
		TCPAttack,
		UDPAttack
	};
	Informator();
	
	bool LoadNewData() noexcept;

	std::optional<std::vector<std::pair<std::string, std::string>>> GetProxies() const noexcept;
	std::optional<std::string> GetTarget() const noexcept;
	std::optional<AttackMethod> GetMethod() const noexcept;

	inline bool IsValid() const noexcept
	{
		return m_isValid;
	}

private:
	static void LoadResouces() noexcept;
private:
	bool m_isValid{false};

	static std::vector<std::string> m_availableResources;
	static std::once_flag m_loadResourcesFlag;

	nlohmann::json m_data;
};


#endif // API_INTERFACE_H
