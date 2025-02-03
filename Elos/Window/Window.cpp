#include <Elos/Window/Window.h>
#include <Elos/Common/String.h>

namespace Elos
{	
	u32 Window::s_windowCount = 0;
	const wchar_t* Window::s_className = L"ElosWindowClass";

	Window::Window(const String& title, const WindowSize& size, const WindowStyle& style)
	{
		Create(title, size, style);
	}
	
	Window::~Window()
	{
		Close();
	}
	
	Window::Window(Window&& other) noexcept
		: m_handle(other.m_handle)
		, m_size(other.m_size)
		, m_minimumSize(other.m_minimumSize)
		, m_events(std::move(other.m_events))
		, m_cursorVisible(other.m_cursorVisible)
		, m_cursorLocked(other.m_cursorLocked)
		, m_keyRepeatEnabled(other.m_keyRepeatEnabled)
		, m_mouseInside(other.m_mouseInside)
		, m_cursor(other.m_cursor)
	{
		other.m_handle = nullptr;
	}
	
	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other)
		{
			Close();

			m_handle           = other.m_handle;
			m_size             = other.m_size;
			m_minimumSize      = other.m_minimumSize;
			m_events           = std::move(other.m_events);
			m_cursorVisible    = other.m_cursorVisible;
			m_cursorLocked     = other.m_cursorLocked;
			m_keyRepeatEnabled = other.m_keyRepeatEnabled;
			m_mouseInside      = other.m_mouseInside;
			m_cursor           = other.m_cursor;

			other.m_handle = nullptr;
		}

		return *this;
	}

	void Window::Create(const String& title, const WindowSize& size, const WindowStyle& style)
	{
		Close();

		if (s_windowCount == 0)
		{
			RegisterWindowClass();
		}

		// Compute screen position
		const HDC screenDC = ::GetDC(nullptr);
		const i32 left     = ::GetDeviceCaps(screenDC, HORZRES) / 2 - static_cast<i32>(size.Width) / 2;
		const i32 top      = ::GetDeviceCaps(screenDC, VERTRES) / 2 - static_cast<i32>(size.Height) / 2;
		::ReleaseDC(nullptr, screenDC);

		// Compute window style
		DWORD win32Style = WS_VISIBLE;
		if (style == WindowStyle::None)
		{
			win32Style |= WS_POPUP;
		}
		else
		{
			if ((style & WindowStyle::Titlebar) == WindowStyle::Titlebar)
				win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
			if ((style & WindowStyle::Resize) == WindowStyle::Resize)
				win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
			if ((style & WindowStyle::Close) == WindowStyle::Close)
				win32Style |= WS_SYSMENU;
		}

		const WindowSize& windowSize = ContentSizeToWindowSize(size);

		m_handle = ::CreateWindowW(
			s_className,
			StringToWString(title).c_str(),
			win32Style,
			left, top,
			size.Width, size.Height,
			nullptr, nullptr,
			::GetModuleHandle(nullptr),
			this);

		SetWindowLongPtrW(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

		SetVisible(true);
		SetMouseCursorVisible(true);
		SetKeyRepeatEnabled(true);

		++s_windowCount;
	}
	
	NODISCARD bool Window::IsOpen() const
	{
		return m_handle != nullptr;
	}
	
	NODISCARD WindowPosition Window::GetPosition() const
	{
		RECT rect;
		::GetWindowRect(m_handle, &rect);
		return { static_cast<i32>(rect.left), static_cast<i32>(rect.top) };
	}
	
	NODISCARD WindowSize Window::GetSize() const
	{
		return m_size;
	}
	
	NODISCARD WindowHandle Window::GetHandle() const
	{
		return NODISCARD WindowHandle();
	}
	
	void Window::Close()
	{
		if (m_handle)
		{
			::DestroyWindow(m_handle);
			m_handle = nullptr;

			--s_windowCount;

			if (s_windowCount == 0)
			{
				::UnregisterClassW(s_className, ::GetModuleHandle(nullptr));
			}
		}
	}
	
	void Window::SetPosition(const WindowPosition& position)
	{
		::SetWindowPos(m_handle, nullptr, position.X, position.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
	
	void Window::SetSize(const WindowSize& size)
	{
		const WindowSize& windowSize = ContentSizeToWindowSize(size);
		::SetWindowPos(m_handle, nullptr, 0, 0, windowSize.Width, windowSize.Height, SWP_NOMOVE | SWP_NOZORDER);
		m_size = size;
	}
	
	void Window::SetMinimumSize(const WindowSize& size)
	{
	}
		
	void Window::SetTitle(const String& title)
	{
		::SetWindowTextW(m_handle, StringToWString(title).c_str());
	}
	
	void Window::SetVisible(bool visible)
	{
		::ShowWindow(m_handle, visible ? SW_SHOW : SW_HIDE);
	}
	
	void Window::RequestFocus()
	{
		DWORD thisPid, foregroundPid;
		::GetWindowThreadProcessId(m_handle, &thisPid);
		::GetWindowThreadProcessId(GetForegroundWindow(), &foregroundPid);

		if (thisPid == foregroundPid)
		{
			::SetForegroundWindow(m_handle);
		}
	}
	
	bool Window::HasFocus() const
	{
		return m_handle == ::GetForegroundWindow();
	}
	
	void Window::SetMouseCursorVisible(bool visible)
	{
		m_cursorVisible = visible;
		::SetCursor(m_cursorVisible ? m_cursor : nullptr);
	}
	
	void Window::SetMouseCursorLocked(bool locked)
	{
		m_cursorLocked = locked;

		if (m_cursorLocked)
		{
			RECT rect;
			::GetClientRect(m_handle, &rect);
			::MapWindowPoints(m_handle, nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
			::ClipCursor(&rect);
		}
		else
		{
			::ClipCursor(nullptr);
		}
	}
	
	void Window::SetKeyRepeatEnabled(bool enabled)
	{
		m_keyRepeatEnabled = enabled;
	}
	
	std::optional<Event> Window::PollEvent()
	{
		ProcessEvents();

		if (!m_events.empty())
		{
			Event event = m_events.front();
			m_events.pop();
			return event;
		}

		return std::nullopt;
	}
	
	void Window::RegisterWindowClass()
	{
		WNDCLASSW windowClass     = { 0 };
		windowClass.style         = 0;
		windowClass.lpfnWndProc   = &Window::GlobalOnEvent;
		windowClass.cbClsExtra    = 0;
		windowClass.cbWndExtra    = 0;
		windowClass.hInstance     = GetModuleHandleW(nullptr);
		windowClass.hIcon         = nullptr;
		windowClass.hCursor       = nullptr;
		windowClass.hbrBackground = nullptr;
		windowClass.lpszMenuName  = nullptr;
		windowClass.lpszClassName = s_className;
		::RegisterClassW(&windowClass);
	}
	
	LRESULT Window::GlobalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* window = handle ? reinterpret_cast<Window*>(::GetWindowLongPtr(handle, GWLP_USERDATA)) : nullptr;

		if (window)
		{
			window->ProcessEvent(message, wParam, lParam);
		}

		if (message == WM_CLOSE)
		{
			return 0;
		}

		// Don't forward the menu system command, so that pressing ALT or F10 doesn't steal the focus
		if ((message == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
		{
			return 0;
		}

		return ::DefWindowProc(handle, message, wParam, lParam);
	}
	
	void Window::ProcessEvent(UINT msg, WPARAM wParam, LPARAM lParam)
	{
	}
	
	void Window::PushEvent(const Event& event)
	{
		m_events.push(event);
	}
	
	void Window::ProcessEvents()
	{
		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
	}
	
	WindowSize Window::ContentSizeToWindowSize(const WindowSize& size) const
	{
		RECT rectangle = { 0, 0, static_cast<LONG>(size.Width), static_cast<LONG>(size.Height) };
		::AdjustWindowRect(&rectangle, static_cast<DWORD>(GetWindowLongPtr(m_handle, GWL_STYLE)), false);
		return { static_cast<u32>(rectangle.right - rectangle.left), static_cast<u32>(rectangle.bottom - rectangle.top) };
	}
}