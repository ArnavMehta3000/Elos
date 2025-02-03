#pragma once

#include <Elos/Export.h>
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>
#include <Elos/Window/WindowHandle.h>
#include <Elos/Window/WindowTypes.h>
#include <Elos/Window/Event.h>
#include <Elos/Window/Input/Keyboard.h>
#include <Elos/Window/Input/Mouse.h>
#include <optional>
#include <queue>
#include <memory>
#include <Windows.h>

namespace Elos
{
	class ELOS_API Window
	{
	public:
		Window() = default;
		Window(const String& title, const WindowSize& size, const WindowStyle& style = WindowStyle::Default);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&& other) noexcept;
		Window& operator=(Window&&other ) noexcept;

		void Create(const String& title, const WindowSize& size, const WindowStyle& style = WindowStyle::Default);

		NODISCARD bool IsOpen() const;
		NODISCARD WindowPosition GetPosition() const;
		NODISCARD WindowSize GetSize() const;
		NODISCARD WindowHandle GetHandle() const;
		
		void Close();
		void SetPosition(const WindowPosition& position) const;
		void SetSize(const WindowSize& size);
		void SetMinimumSize(const WindowSize& size);
		void SetTitle(const String& title) const;
		void SetVisible(bool visible);
		void RequestFocus() const;
		bool HasFocus() const;
		
		std::optional<Event> PollEvent();

	private:
		static void RegisterWindowClass();
		static LRESULT CALLBACK GlobalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
		
		void RegisterRawInputDevices() const;
		void ProcessEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		void PushEvent(const Event& event);
		void ProcessEvents();

		WindowSize ContentSizeToWindowSize(const WindowSize& size) const;

	private:
		WindowHandle      m_handle{ nullptr };
		WindowSize        m_size{ 0, 0 };
		WindowSize        m_minimumSize{ 20, 20 };
		std::queue<Event> m_events;
		char16            m_surrogate{ 0 };

		std::unique_ptr<Keyboard> m_keyboard;
		std::unique_ptr<Mouse> m_mouse;

		static u32            s_windowCount;
		static const wchar_t* s_className;
	};
}