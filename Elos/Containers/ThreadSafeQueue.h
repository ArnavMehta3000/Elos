#pragma once
#include <Elos/Common/FunctionMacros.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace Elos
{
	template<typename T>
	class ThreadSafeQueue
	{
	public:
		ThreadSafeQueue() = default;
		~ThreadSafeQueue() = default;

		ThreadSafeQueue(const ThreadSafeQueue&) = delete;
		ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
		ThreadSafeQueue(ThreadSafeQueue&&) = delete;
		ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

		void Push(const T& item)
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_queue.push(item);
			}
			m_condVar.notify_one();
		}

		NODISCARD std::optional<T> TryPop()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_queue.empty())
			{
				return std::nullopt;
			}

			T item = m_queue.front();
			m_queue.pop();
			return item;
		}

		T WaitAndPop()
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_condVar.wait(lock, [this] { return !m_queue.empty(); });

			T item = m_queue.front();
			m_queue.pop();
			return item;
		}

		bool Empty() const
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_queue.empty();
		}

		size_t Size() const
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_queue.size();
		}

		void Clear()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			std::queue<T> empty;
			std::swap(m_queue, empty);
		}

	private:
		std::queue<T>           m_queue;
		mutable std::mutex      m_mutex;
		std::condition_variable m_condVar;
	};
}