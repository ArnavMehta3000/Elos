#include <Elos/Application/AppBase.h>

namespace Elos
{
	AppBase::AppBase()
	{
		// Create window
		WindowCreateInfo info{};
		GetWindowCreateInfo(info);

		m_window = std::make_unique<Window>(info);
	}

	void AppBase::ProcessWindowEvents()
	{
		// Bind these lambdas to fire off the signals
		static const auto OnWindowClosed              = [this](const Event::Closed& e)              { m_windowEventSignals.OnClosed.Emit(e);              };
		static const auto OnWindowFocusLost           = [this](const Event::FocusLost& e)           { m_windowEventSignals.OnFocusLost.Emit(e);           };
		static const auto OnWindowFocusGained         = [this](const Event::FocusGained& e)         { m_windowEventSignals.OnFocusGained.Emit(e);         };
		static const auto OnWindowMouseEntered        = [this](const Event::MouseEntered& e)        { m_windowEventSignals.OnMouseEntered.Emit(e);        };
		static const auto OnWindowMouseLeft           = [this](const Event::MouseLeft& e)           { m_windowEventSignals.OnMouseLeft.Emit(e);           };
		static const auto OnWindowResized             = [this](const Event::Resized& e)             { m_windowEventSignals.OnResized.Emit(e);             };
		static const auto OnWindowTextInput           = [this](const Event::TextInput& e)           { m_windowEventSignals.OnTextInput.Emit(e);           };
		static const auto OnWindowKeyPressed          = [this](const Event::KeyPressed& e)          { m_windowEventSignals.OnKeyPressed.Emit(e);          };
		static const auto OnWindowKeyReleased         = [this](const Event::KeyReleased& e)         { m_windowEventSignals.OnKeyReleased.Emit(e);         };
		static const auto OnWindowMouseWheelScrolled  = [this](const Event::MouseWheelScrolled& e)  { m_windowEventSignals.OnMouseWheelScrolled.Emit(e);  };
		static const auto OnWindowMouseButtonPressed  = [this](const Event::MouseButtonPressed& e)  { m_windowEventSignals.OnMouseButtonPressed.Emit(e);  };
		static const auto OnWindowMouseButtonReleased = [this](const Event::MouseButtonReleased& e) { m_windowEventSignals.OnMouseButtonReleased.Emit(e); };
		static const auto OnWindowMouseMoved          = [this](const Event::MouseMoved& e)          { m_windowEventSignals.OnMouseMoved.Emit(e);          };
		static const auto OnWindowMouseMovedRaw       = [this](const Event::MouseMovedRaw& e)       { m_windowEventSignals.OnMouseMovedRaw.Emit(e);       };
		
		m_window->HandleEvents(
			OnWindowClosed,
			OnWindowFocusLost,
			OnWindowFocusGained,
			OnWindowMouseEntered,
			OnWindowMouseLeft,
			OnWindowResized,
			OnWindowTextInput,
			OnWindowKeyPressed,
			OnWindowKeyReleased,
			OnWindowMouseWheelScrolled,
			OnWindowMouseButtonPressed,
			OnWindowMouseButtonReleased,
			OnWindowMouseMoved,
			OnWindowMouseMovedRaw
		);
	}

	AppBase::~AppBase()
	{
		m_windowEventSignals.DisconnectAll();
	}

	void AppBase::GetWindowCreateInfo(WindowCreateInfo& outCreateInfo)
	{
		outCreateInfo = WindowCreateInfo::Default("Elos App", { 1280, 720 });
	}
}