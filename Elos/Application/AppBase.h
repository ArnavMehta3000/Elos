#pragma once
#include <Elos/Window/Window.h>

namespace Elos
{
	/**
	 * @brief Application base that contains a window and assists in routing window events
	 */
	class ELOS_API AppBase
	{
	public:
		virtual ~AppBase();
		
		virtual void GetWindowCreateInfo(WindowCreateInfo& outCreateInfo);

		template <typename EventType>
		void SetUpConnection(WindowEventConnections& connections, const std::function<void(EventType)>& func)
		{
			static_assert(false, "AppBase::SetUpConnection not specialized for non-event types");
		}

	protected:
		AppBase();
		void ProcessWindowEvents();

	protected:
		std::unique_ptr<Window> m_window;
		WindowEventSignals m_windowEventSignals{};
	};

	// Macro to set up window event connections template specializations
#define Elos_SetUpConnectionFunc(EventName)                                           \
	template<>                                                                        \
	inline void AppBase::SetUpConnection<const Event::EventName&>(                    \
		WindowEventConnections& connections,                                          \
		const std::function<void(const Event::EventName&)>& func)                     \
	{                                                                                 \
		connections.On##EventName = m_windowEventSignals.On##EventName.Connect(func); \
	}

	Elos_SetUpConnectionFunc(Closed)
	Elos_SetUpConnectionFunc(FocusLost)
	Elos_SetUpConnectionFunc(FocusGained)
	Elos_SetUpConnectionFunc(MouseEntered)
	Elos_SetUpConnectionFunc(MouseLeft)
	Elos_SetUpConnectionFunc(Resized)
	Elos_SetUpConnectionFunc(TextInput)
	Elos_SetUpConnectionFunc(KeyPressed)
	Elos_SetUpConnectionFunc(KeyReleased)
	Elos_SetUpConnectionFunc(MouseWheelScrolled)
	Elos_SetUpConnectionFunc(MouseButtonPressed)
	Elos_SetUpConnectionFunc(MouseButtonReleased)
	Elos_SetUpConnectionFunc(MouseMoved)
	Elos_SetUpConnectionFunc(MouseMovedRaw)
#undef Elos_SetUpConnectionFunc
}