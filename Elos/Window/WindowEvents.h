#pragma once

#include <Elos/Common/FunctionMacros.h>
#include <Elos/Window/WindowTypes.h>
#include <Elos/Window/Input/KeyCode.h>
#include <Elos/Event/Signal.h>
#include <variant>
#include <type_traits>

namespace Elos
{
	class Event
	{
	public:
		struct Created{};
		struct Closed{};
		struct FocusLost{};
		struct FocusGained{};
		struct MouseEntered{};
		struct MouseLeft{};

		struct Resized
		{
			WindowSize Size;
		};

		struct TextInput
		{
			char32 UnicodeChar;

			constexpr char AsChar() const
			{
				return UnicodeChar <= 0x7F ? static_cast<char>(UnicodeChar) : '?';
			}
		};

		struct KeyPressed
		{
			KeyCode::Key Key;
			bool Alt;
			bool Control;
			bool Shift;
			bool System;
		};

		struct KeyReleased
		{
			KeyCode::Key Key;
			bool Alt;
			bool Control;
			bool Shift;
			bool System;
		};

		struct MouseWheelScrolled
		{
			KeyCode::MouseWheel Wheel;
			f32 Delta;
			i32 X;
			i32 Y;
		};

		struct MouseButtonPressed
		{
			KeyCode::MouseButton Button;
			i32 X;
			i32 Y;
		};

		struct MouseButtonReleased
		{
			KeyCode::MouseButton Button;
			i32 X;
			i32 Y;
		};

		struct MouseMoved
		{
			i32 X;
			i32 Y;
		};
		
		struct MouseMovedRaw
		{
			i32 DeltaX;
			i32 DeltaY;
		};

	private:
		using EventVariant = std::variant<
			Closed,
			FocusLost,
			FocusGained,
			MouseEntered,
			MouseLeft,
			Resized,
			TextInput,
			KeyPressed,
			KeyReleased,
			MouseWheelScrolled,
			MouseButtonPressed,
			MouseButtonReleased,
			MouseMoved,
			MouseMovedRaw>;

		template<typename T>
		static constexpr bool IsEventType = std::disjunction_v<
			std::is_same<T, Closed>,
			std::is_same<T, Resized>,
			std::is_same<T, FocusLost>,
			std::is_same<T, FocusGained>,
			std::is_same<T, TextInput>,
			std::is_same<T, KeyPressed>,
			std::is_same<T, KeyReleased>,
			std::is_same<T, MouseWheelScrolled>,
			std::is_same<T, MouseButtonPressed>,
			std::is_same<T, MouseButtonReleased>,
			std::is_same<T, MouseMoved>,
			std::is_same<T, MouseMovedRaw>,
			std::is_same<T, MouseEntered>,
			std::is_same<T, MouseLeft>>;

	public:
		template<typename T>
		Event(const T& eventData) : m_eventData(eventData)
		{
			static_assert(IsEventType<T>, "Invalid event type");
		}

		// Type checking
		template<typename T>
		NODISCARD bool Is() const
		{
			static_assert(IsEventType<T>, "Invalid event type");
			return std::holds_alternative<T>(m_eventData);
		}

		// Get event data
		template<typename T>
		NODISCARD const T* Get() const
		{
			static_assert(IsEventType<T>, "Invalid event type");
			return std::get_if<T>(&m_eventData);
		}

		// Visit pattern
		template<typename Visitor>
		decltype(auto) visit(Visitor&& visitor)
		{
			return std::visit(std::forward<Visitor>(visitor), m_eventData);
		}

		template<typename Visitor>
		decltype(auto) visit(Visitor&& visitor) const
		{
			return std::visit(std::forward<Visitor>(visitor), m_eventData);
		}

	private:
		EventVariant m_eventData;
	};

	struct WindowEventSignals
	{
		Signal<const Event::Closed&>              OnClosed;
		Signal<const Event::FocusLost&>           OnFocusLost;
		Signal<const Event::FocusGained&>         OnFocusGained;
		Signal<const Event::MouseEntered&>        OnMouseEntered;
		Signal<const Event::MouseLeft&>           OnMouseLeft;
		Signal<const Event::Resized&>             OnResized;
		Signal<const Event::TextInput&>           OnTextInput;
		Signal<const Event::KeyPressed&>          OnKeyPressed;
		Signal<const Event::KeyReleased&>         OnKeyReleased;
		Signal<const Event::MouseWheelScrolled&>  OnMouseWheelScrolled;
		Signal<const Event::MouseButtonPressed&>  OnMouseButtonPressed;
		Signal<const Event::MouseButtonReleased&> OnMouseButtonReleased;
		Signal<const Event::MouseMoved&>          OnMouseMoved;
		Signal<const Event::MouseMovedRaw&>       OnMouseMovedRaw;

		void DisconnectAll()
		{
			OnClosed.DisconnectAll();
			OnFocusLost.DisconnectAll();
			OnFocusGained.DisconnectAll();
			OnMouseEntered.DisconnectAll();
			OnMouseLeft.DisconnectAll();
			OnResized.DisconnectAll();
			OnTextInput.DisconnectAll();
			OnKeyPressed.DisconnectAll();
			OnKeyReleased.DisconnectAll();
			OnMouseWheelScrolled.DisconnectAll();
			OnMouseButtonPressed.DisconnectAll();
			OnMouseButtonReleased.DisconnectAll();
			OnMouseMoved.DisconnectAll();
			OnMouseMovedRaw.DisconnectAll();
		}
	};

	struct WindowEventConnections
	{
		Connection<const Event::Closed&>              OnClosed;
		Connection<const Event::FocusLost&>           OnFocusLost;
		Connection<const Event::FocusGained&>         OnFocusGained;
		Connection<const Event::MouseEntered&>        OnMouseEntered;
		Connection<const Event::MouseLeft&>           OnMouseLeft;
		Connection<const Event::Resized&>             OnResized;
		Connection<const Event::TextInput&>           OnTextInput;
		Connection<const Event::KeyPressed&>          OnKeyPressed;
		Connection<const Event::KeyReleased&>         OnKeyReleased;
		Connection<const Event::MouseWheelScrolled&>  OnMouseWheelScrolled;
		Connection<const Event::MouseButtonPressed&>  OnMouseButtonPressed;
		Connection<const Event::MouseButtonReleased&> OnMouseButtonReleased;
		Connection<const Event::MouseMoved&>          OnMouseMoved;
		Connection<const Event::MouseMovedRaw&>       OnMouseMovedRaw;

		void DisconnectAll()
		{
			OnClosed.Disconnect();
			OnFocusLost.Disconnect();
			OnFocusGained.Disconnect();
			OnMouseEntered.Disconnect();
			OnMouseLeft.Disconnect();
			OnResized.Disconnect();
			OnTextInput.Disconnect();
			OnKeyPressed.Disconnect();
			OnKeyReleased.Disconnect();
			OnMouseWheelScrolled.Disconnect();
			OnMouseButtonPressed.Disconnect();
			OnMouseButtonReleased.Disconnect();
			OnMouseMoved.Disconnect();
			OnMouseMovedRaw.Disconnect();
		}
	};
}