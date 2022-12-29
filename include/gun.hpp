#ifndef GUN_H
#define GUN_H

#include <optional>
#include <string>

#include "target.hpp"
#include "multithread.h"
#include "uri.h"

namespace Attackers
{
	class IGun;
	using PGun = std::unique_ptr<IGun>;

	// TODO: can think of optimization with fixed array 
	// for on stack allocation of new child of IGun
	class IGun
	{
	public:
		IGun(const TaskController &task) : m_currentTask{task}
		{ }
		virtual ~IGun() {}

		virtual std::size_t FireTillDead(const URI &targetToKill) noexcept = 0;
	protected:
		const TaskController &m_currentTask;
	};

} // Attackers

#endif // GUN_H
