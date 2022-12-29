#ifndef STATIC_TARGET_H
#define STATIC_TARGET_H

#include "target.hpp"

namespace Attackers
{
	class StaticTarget : public Target
	{
	public:
		StaticTarget(const URI &coords, AttackMethod method) :
			Target{false, coords, method}
		{ }

		~StaticTarget() override
		{ }

		void Retarget(const TaskController &) override
		{}

		PTarget Clone() override
		{
			return std::make_unique<StaticTarget>(*this);
		}
	};
}

#endif // STATIC_TARGET_H
