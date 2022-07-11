#ifndef DYNAMIC_TARGET_H
#define DYNAMIC_TARGET_H

#include "target.hpp"
#include "api_interface.h"

namespace Attackers
{
	class DynamicTarget : public Target
	{
	public:
		DynamicTarget();
		~DynamicTarget() override = default;

		void Retarget(const TaskController &task) override;

		PTarget Clone() override;
	private:
		void SetNewTarget(const TaskController &task);
	
	private:
		Informator m_informator;
	};
} // namespace Attackers

#endif // DYNAMIC_TARGET_H
