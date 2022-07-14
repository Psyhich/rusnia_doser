#include "multithread.h"

TaskController::~TaskController()
{
	SendStop();
	UtilizeTask();
}

void TaskController::UtilizeTask() noexcept
{
	if(m_currentThread && m_currentThread->joinable())
	{
		m_currentThread->join();
	}
}
