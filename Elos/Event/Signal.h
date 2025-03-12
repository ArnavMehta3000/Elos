#pragma once
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/FunctionMacros.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include <ranges>

namespace Elos
{
	template <typename... Args> class Connection;

	template <typename F, typename... Args>
	concept Invocable = requires(F f, Args... args) 
	{
		{ std::invoke(f, args...) };
	};

	template <typename... Args>
	class Signal
	{
		friend class Connection<Args...>;

	public:
		using SlotFunction = std::function<void(Args...)>;
		using ConnectionId = u32;

		Signal() = default;
		~Signal() = default;
		Signal(Signal&&) noexcept = default;
		Signal(const Signal&) = delete;
		Signal& operator=(Signal&&) noexcept = default;
		Signal& operator=(const Signal&) = delete;

		template <Invocable<Args...> Callable>
		NODISCARD Connection<Args...> Connect(Callable&& callable)
		{
			static ConnectionId nextId = 0;
			ConnectionId id = ++nextId;

			std::shared_ptr<SlotInfo> info = std::make_shared<SlotInfo>();
			info->Id = id;
			info->Function = std::forward<Callable>(callable);

			m_slots.push_back(info);

			return Connection<Args...>(std::move(info), this);
		}

		NODISCARD bool Disconnect(ConnectionId id)
		{
			auto it = std::ranges::find_if(m_slots, [id](const auto& slot) 
			{
				return slot->Id == id;
			});

			if (it != m_slots.end())
			{
				(*it)->ParentSignal = nullptr;
				m_slots.erase(it);
				return true;
			}
			return false;
		}

		void DisconnectAll() noexcept
		{
			for (auto& slot : m_slots)
			{
				slot->ParentSignal = nullptr;
			}
			m_slots.clear();
		}

		void Emit(Args... args) const
		{
			for (const auto& slot : m_slots)
			{
				slot->Function(args...);
			}
		}

		NODISCARD u64 ConnectionCount() const noexcept { return m_slots.size(); }
		NODISCARD bool HasConnections() const noexcept { return !m_slots.empty();}

		void operator()(Args... args) const
		{
			Emit(std::forward<Args>(args)...);
		}

	private:
		struct SlotInfo
		{
			ConnectionId Id;
			SlotFunction Function;
			Signal<Args...>* ParentSignal = nullptr;
		};

		std::vector<std::shared_ptr<SlotInfo>> m_slots;
	};

	template <typename... Args>
	class Connection
	{
	public:
		Connection() = default;
		
		Connection(std::shared_ptr<typename Signal<Args...>::SlotInfo> slotInfo, Signal<Args...>* signal)
			: m_slotInfo(std::move(slotInfo))
		{
			if (m_slotInfo)
			{
				m_slotInfo->ParentSignal = signal;
			}
		}
		
		Connection(Connection&&) noexcept = default;
		Connection(const Connection&) = default;
		~Connection() = default;
		Connection& operator=(Connection&&) noexcept = default;
		Connection& operator=(const Connection&) = default;

		bool Disconnect()
		{
			if (m_slotInfo && m_slotInfo->ParentSignal)
			{
				auto* signal = m_slotInfo->ParentSignal;
				auto id = m_slotInfo->Id;

				m_slotInfo->ParentSignal = nullptr;
				m_slotInfo.reset();
				return signal->Disconnect(id);
			}
			return false;
		}

		NODISCARD bool IsConnected() const noexcept
		{
			return m_slotInfo && m_slotInfo->ParentSignal;
		}

		NODISCARD auto Id() const noexcept
		{
			return m_slotInfo ? m_slotInfo->Id : 0;
		}

	private:
		std::shared_ptr<typename Signal<Args...>::SlotInfo> m_slotInfo;
	};

	template <typename... Args>
	class ScopedConnection : public Connection<Args...>
	{
	public:
		using Connection<Args...>::Connection;

		ScopedConnection(Connection<Args...>&& conn) noexcept
			: Connection<Args...>(std::move(conn)) {}
		ScopedConnection(ScopedConnection&&) noexcept = default;
		ScopedConnection(const ScopedConnection&) = delete;
		
		~ScopedConnection() 
		{
			this->Disconnect();
		}

		ScopedConnection& operator=(ScopedConnection&&) noexcept = default;
		ScopedConnection& operator=(const ScopedConnection&) = delete;
	};
} 