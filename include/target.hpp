#ifndef TARGET_RUSNIA_DOSER_HPP
#define TARGET_RUSNIA_DOSER_HPP

#include <iterator>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "multithread.h"
#include "uri.h"

namespace Attackers
{
	enum class AttackMethod;

	struct Tactic
	{
		AttackMethod method;
		URI coordintates{""};
		std::size_t squadSize;
	};

	enum class AttackMethod
	{
		HTTPAttack,
		TCPAttack,
		UDPAttack
	};

	class Target;
	using PTarget = std::unique_ptr<Target>;

	class Target
	{
	public:
		Target(bool needWeaponAim) :
			m_needWeaponAim{needWeaponAim}
		{ }

		Target(bool needWeaponAim, const URI &coords, AttackMethod method) :
			m_coordinates{coords},
			m_method{method},
			m_needWeaponAim{needWeaponAim}
		{ }
		virtual ~Target()
		{ }

		virtual void Retarget(const TaskController &task) = 0;

		inline const URI &GetCoordinates() const noexcept
		{
			return m_coordinates;
		}

		inline AttackMethod GetAttackMethod() const noexcept
		{
			return m_method;
		}

		inline bool NeedWeaponAim() const noexcept
		{
			return m_needWeaponAim;
		}

		virtual PTarget Clone() = 0;

	protected:
		inline void SetCordinates(const URI &coordinatesToSet)
		{
			m_coordinates = coordinatesToSet;
		}

		inline void SetAttackMethod(AttackMethod method)
		{
			m_method = method;
		}

		inline void SetNeedForWeaponAim(bool needAim)
		{
			m_needWeaponAim = needAim;
		}

	private:
		URI m_coordinates;
		AttackMethod m_method;

		bool m_needWeaponAim;
	};
}

template<typename OStream>
OStream &operator<<(OStream &os, const Attackers::AttackMethod& method)
{
	using namespace Attackers;

	std::string methodName;
	switch (method)
	{
		case AttackMethod::HTTPAttack:
		{
			methodName = "HTTP";
			break;
		}
		case AttackMethod::TCPAttack:
		{
			methodName = "TCP";
			break;
		}
		case AttackMethod::UDPAttack:
		{
			methodName = "UDP";
			break;
		}
		default:
		{
			methodName = "Unknown";
			break;
		}
	}
	return os << methodName;
}


#endif // TARGET_RUSNIA_DOSER_HPP
