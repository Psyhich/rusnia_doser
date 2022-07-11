#include "spdlog/spdlog.h"

#include "dynamic_target.h"

using namespace Attackers;

void DynamicTarget::SetNewTarget(const TaskController &task)
{
	while(!task.ShouldStop())
	{
		if(!m_informator.LoadNewData())
		{
			continue;
		}

		if(m_informator.GetTarget() && m_informator.GetMethod())
		{
			SetCordinates(*m_informator.GetTarget());
			SetAttackMethod(*m_informator.GetMethod());
			break;
		}
	}
}

DynamicTarget::DynamicTarget() : 
	Target{true}
{
}

void DynamicTarget::Retarget(const TaskController &task)
{
	SetNewTarget(task);
}

PTarget DynamicTarget::Clone()
{
	return std::make_unique<DynamicTarget>();
}
