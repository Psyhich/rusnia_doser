#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include <atomic>

class TaskController
{
public:
	TaskController() = default;

	TaskController(const TaskController &copy) = delete;
	TaskController &operator=(const TaskController &copy) = delete;

	TaskController(TaskController &&move) = delete;
	TaskController &operator=(const TaskController &&move) = delete;


	inline bool ShouldStop() const
	{
		return m_shouldStop.load(std::memory_order_acquire);
	}
	inline void StopExecution()
	{
		m_shouldStop.store(true, std::memory_order_release);
	}
	inline void StartExecution()
	{
		m_shouldStop.store(false, std::memory_order_release);
	}
private:
	std::atomic<bool> m_shouldStop{true};
};

#endif // MULTITHREAD_H
