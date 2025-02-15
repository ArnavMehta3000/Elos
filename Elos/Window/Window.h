#pragma once

#include <Elos/Export.h>
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>
#include <Elos/Window/Input/Keyboard.h>
#include <Elos/Window/Input/Mouse.h>
#include <Elos/Window/WindowEvents.h>
#include <Elos/Window/WindowHandle.h>
#include <Elos/Window/WindowTypes.h>
#include <memory>
#include <optional>
#include <queue>
#include <Windows.h>

// Use common controls
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
				processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace Elos
{

	namespace Internal
	{
		// Helper class for event handlers
		class WindowEventHandlerDispatcher
		{
		private:
			// Default handler that does nothing for unhandled event types
			struct DefaultHandler
			{
				void operator()(const auto&) const {}
			};

			// Unpack handlers into a combined callable
			template<typename... Handlers>
			struct OverloadSet : DefaultHandler, std::decay_t<Handlers>...
			{
				using DefaultHandler::operator();
				using std::decay_t<Handlers>::operator()...;

				explicit OverloadSet(Handlers&&... handlers)
					: std::decay_t<Handlers>(std::forward<Handlers>(handlers))... {}
			};

		public:
			template<typename... Handlers>
			static void Dispatch(Window& window, Handlers&&... handlers);
		};
	};

	class ELOS_API Window
	{
	public:
		Window() = default;
		Window(const WindowCreateInfo& info);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&& other) noexcept;
		Window& operator=(Window&&other ) noexcept;

		NODISCARD Window* CreateChild(const WindowCreateInfo& createInfo);

		NODISCARD bool IsOpen() const;
		NODISCARD WindowPosition GetPosition() const;
		NODISCARD WindowSize GetSize() const;
		NODISCARD WindowHandle GetHandle() const;
		
		void Close();
		void SetPosition(const WindowPosition& position) const;
		void SetSize(const WindowSize& size);
		void SetMinimumSize(const WindowSize& size);
		void SetTitle(const String& title) const;
		void SetVisible(bool visible) const;
		void RequestFocus() const;
		bool HasFocus() const;
		
		std::optional<Event> PollEvent();

		template <typename... Handlers>
		void HandleEvents(Handlers&&... handlers);

		NODISCARD bool IsChild() const { return m_childMode != WindowChildMode::None; }
		NODISCARD WindowChildMode GetChildMode() const { return m_childMode; }
		NODISCARD Window* GetParent() const { return m_parent; }
		NODISCARD const std::vector<Window>& GetChildren() const { return m_children; }

	private:
		void Create(const WindowCreateInfo& createInfo);
		static void RegisterWindowClass();
		static LRESULT CALLBACK GlobalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
		
		void RegisterRawInputDevices() const;
		void SetDPIAwareness() const;
		NODISCARD LRESULT ProcessEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		void PushEvent(const Event& event);
		void PumpMessages();

		WindowSize ContentSizeToWindowSize(const WindowSize& size) const;

		NODISCARD Window* AddChild(const WindowCreateInfo& createInfo);
		void RemoveChild(Window* child);
		NODISCARD DWORD GetWin32WindowStyle(WindowStyle style, WindowChildMode childMode) const;
		void UpdateChildWindowStyles();

	private:
		WindowHandle              m_handle{ nullptr };
		WindowSize                m_size{ 0, 0 };
		WindowSize                m_minimumSize{ 20, 20 };
		char16                    m_surrogate{ 0 };
		WindowChildMode           m_childMode{ WindowChildMode::None };
		std::unique_ptr<Keyboard> m_keyboard;
		std::unique_ptr<Mouse>    m_mouse;
		Window*                   m_parent{ nullptr };
		std::queue<Event>         m_events;
		std::vector<Window>       m_children;

		static u32            s_windowCount;
		static const wchar_t* s_className;
	};

	namespace Internal
	{
		template<typename... Handlers>
		void WindowEventHandlerDispatcher::Dispatch(Window& window, Handlers&&... handlers)
		{
			auto combined = OverloadSet<Handlers...>{ std::forward<Handlers>(handlers)... };

			while (auto event = window.PollEvent())
			{
				event->visit(combined);
			}
		}
	}

	template<typename... Handlers>
	void Window::HandleEvents(Handlers&&... handlers)
	{
		Internal::WindowEventHandlerDispatcher::Dispatch(*this, std::forward<Handlers>(handlers)...);
	}
}