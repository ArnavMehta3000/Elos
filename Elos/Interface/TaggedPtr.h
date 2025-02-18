#pragma once
#include <Elos/Interface/Interface.h>
#include <Elos/Interface/Pack.h>
#include <memory>
#include <variant>
#include <array>
#include <stdexcept>

namespace Elos
{
	template <typename... Types> class TaggedPtr;

	template<typename T>
	concept ValidTaggedType = std::is_class_v<T> && !std::is_abstract_v<T>;

	namespace Internal
	{
		// Concept to check if at least one type implements the interface
		template<typename Interface, typename... Types>
		concept AtLeastOneImplements = (ImplementsInterface<Types, Interface> || ...);

		// Concept to check if a specific type implements the interface
		template<typename T, typename Interface>
		concept TypeImplements = ImplementsInterface<T, Interface>;

		template<typename F, typename T>
		struct GetReturnType { using Type = decltype(std::declval<F>()(std::declval<std::shared_ptr<T>&>())); };

		// Helper struct to handle interface dispatch
		template<typename Interface>
		struct InterfaceDispatcher
		{
			struct DefaultHandler
			{
				void operator()(const auto&) const {}
			};

			template<typename... Handlers>
			struct OverloadSet : DefaultHandler, std::decay_t<Handlers>...
			{
				using DefaultHandler::operator();
				using std::decay_t<Handlers>::operator()...;

				explicit OverloadSet(Handlers&&... handlers)
					: std::decay_t<Handlers>(std::forward<Handlers>(handlers))... {}
			};

			template<typename... Handlers>
			static auto MakeOverloadSet(Handlers&&... handlers)
			{
				return OverloadSet<Handlers...>(std::forward<Handlers>(handlers)...);
			}
		};
	}

	/**
	 * @brief Tagged pointer class that can contain implement various interfaces
	 * @tparam ...Types 
	 */
	template<typename... Types>
	class TaggedPtr
	{
		static_assert((ValidTaggedType<Types> && ...), "All types must be valid tagged types");

	public:
		using VariantType = std::variant<std::shared_ptr<Types>...>;
		using Pack = TypePack<Types...>;

		TaggedPtr() = default;

		template<typename T> requires (HasType<T, Pack>::Value&& ValidTaggedType<T>)
		explicit TaggedPtr(std::shared_ptr<T> ptr) : m_ptr(std::move(ptr)) {}

		template<typename T> requires HasType<T, Pack>::Value
		static constexpr size_t TypeIndex() { return IndexOf<T, Pack>::Count; }

		template<typename T> requires HasType<T, Pack>::Value
		bool Is() const { return std::holds_alternative<std::shared_ptr<T>>(m_ptr); }

		size_t Tag() const { return m_ptr.index(); }

		template<typename T> requires HasType<T, Pack>::Value
		std::shared_ptr<T> Cast()
		{
			if (auto ptr = std::get_if<std::shared_ptr<T>>(&m_ptr))
			{
				return *ptr;
			}
			return nullptr;
		}

		template<typename T> requires HasType<T, Pack>::Value
		std::shared_ptr<const T> Cast() const
		{
			if (auto ptr = std::get_if<std::shared_ptr<T>>(&m_ptr))
			{
				return *ptr;
			}
			return nullptr;
		}

		template<typename Interface, typename F> requires (Internal::AtLeastOneImplements<Interface, Types...>)
		auto Dispatch(F&& func)
		{
			auto handler = Internal::InterfaceDispatcher<Interface>::MakeOverloadSet(
				[&func](auto& ptr) -> decltype(auto)
				{
					using T = std::remove_reference_t<decltype(*ptr)>;
					if constexpr (ImplementsInterface<T, Interface>)
					{
						return func(ptr);
					}
					else
					{
						throw std::runtime_error("Type does not implement interface");
					}
					return;
				}
			);

			return std::visit(handler, m_ptr);
		}

		template<typename Interface, typename F> requires (Internal::AtLeastOneImplements<Interface, Types...>)
		auto Dispatch(F&& func) const
		{
			auto handler = Internal::InterfaceDispatcher<Interface>::MakeOverloadSet(
				[&func](const auto& ptr) -> decltype(auto)
				{
					using T = std::remove_reference_t<decltype(*ptr)>;
					if constexpr (ImplementsInterface<T, Interface>)
{
						return func(ptr);
					}
					else
					{
						throw std::runtime_error("Type does not implement interface");
					}
					return;
				}
			);

			return std::visit(handler, m_ptr);
		}

		explicit operator bool() const
		{
			return std::visit([](const auto& ptr) { return ptr != nullptr; }, m_ptr);
		}

	private:
		VariantType m_ptr;
	};

	// Handle construction using constructor arguments
	template <typename T, typename Pack, typename... Args> requires HasType<T, Pack>::Value&& ValidTaggedType<T>
	auto MakeTagged(Args&&... args)
	{
		using TaggedType = typename Pack::template Expand<TaggedPtr>;
		return TaggedType(std::make_shared<T>(std::forward<Args>(args)...));
	}

	// Handle std::shared_ptr allocation
	template <typename T, typename Pack> requires HasType<T, Pack>::Value&& ValidTaggedType<T>
	auto MakeTagged(std::shared_ptr<T> ptr)
	{
		using TaggedType = typename Pack::template Expand<TaggedPtr>;
		return TaggedType(std::move(ptr));
	}
}