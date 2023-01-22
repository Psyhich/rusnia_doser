#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <thread>
#include <stdexcept>
#include <utility>

class TaskController
{
public:
	TaskController() = default;
	~TaskController();

	TaskController(const TaskController &copy) = delete;
	TaskController &operator=(const TaskController &copy) = delete;

	TaskController(TaskController &&move) = default;
	TaskController &operator=(TaskController &&move) = default;

	inline bool ShouldStop() const noexcept
	{
		return m_shouldStop->load(std::memory_order_acquire);
	}
	inline void SendStop() noexcept
	{
		m_shouldStop->store(true, std::memory_order_release);
	}

	void UtilizeTask() noexcept;

	template<typename Func, typename ...Args> 
	void StartExecution(Func newTask, Args &&...args)
	{
		if(m_currentThread)
		{
			SPDLOG_CRITICAL("Tried to start execution on already running task");
			throw std::runtime_error("Tried to start execution on already running task");
		}

		m_shouldStop->store(false, std::memory_order_release);
		m_currentThread.emplace(std::ref(newTask), std::forward<Args>(args)...);
	}

	inline void StartExecution() noexcept
	{
		m_shouldStop->store(false, std::memory_order_release);
	}
private:
	using AtomicFlag = std::atomic<bool>;
private:
	std::unique_ptr<AtomicFlag> m_shouldStop{std::make_unique<AtomicFlag>(true)};
	std::optional<std::thread> m_currentThread{std::nullopt};
};

#endif // MULTITHREAD_H
