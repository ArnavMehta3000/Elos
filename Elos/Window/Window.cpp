#include <Elos/Window/Window.h>
#include <Elos/Common/String.h>
#include <shellscalingapi.h>
#include <CommCtrl.h>

namespace Elos
{	
	u32 Window::s_windowCount = 0;
	const wchar_t* Window::s_className = L"ElosWindowClass";

	Window::Window(const String& title, const WindowSize& size, const WindowStyle& style)
	{
		// Init common controls
		if (s_windowCount == 0)
		{
			INITCOMMONCONTROLSEX icc = {};
			icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
			bool result = InitCommonControlsEx(&icc);
			int x = 0;
		}

		m_keyboard = std::make_unique<Keyboard>(*this);
		m_mouse = std::make_unique<Mouse>(*this);
		Create(title, size, style);
	}
	
	Window::~Window()
	{
		Close();
		m_keyboard.reset();
		m_mouse.reset();
	}
	
	Window::Window(Window&& other) noexcept
		: m_handle(other.m_handle)
		, m_size(other.m_size)
		, m_minimumSize(other.m_minimumSize)
		, m_events(std::move(other.m_events))
		, m_keyboard(std::move(other.m_keyboard))
		, m_mouse(std::move(other.m_mouse))
	{
		other.m_handle = nullptr;
		other.m_keyboard.reset();
		other.m_mouse.reset();
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
			m_keyboard         = std::move(other.m_keyboard);
			m_mouse            = std::move(other.m_mouse);

			other.m_handle = nullptr;
			other.m_keyboard.reset();
			other.m_mouse.reset();
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

		SetDPIAwareness();

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

		m_handle = ::CreateWindowEx(
			WS_EX_CLIENTEDGE,
			s_className,
			StringToWString(title).c_str(),
			win32Style,
			left, top,
			windowSize.Width, windowSize.Height,
			nullptr, nullptr,
			::GetModuleHandle(nullptr),
			this);

		if (!m_handle)
		{
			return;
		}

		SetWindowLongPtrW(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		RegisterRawInputDevices();

		::ShowWindow(m_handle, SW_SHOW);
		::UpdateWindow(m_handle);
		::SetForegroundWindow(m_handle);
		::SetFocus(m_handle);

		SetVisible(true);
		m_mouse->SetVisible(true);
		m_keyboard->SetKeyRepeatEnabled(true);

		++s_windowCount;
	}
	
	bool Window::IsOpen() const
	{
		return m_handle != nullptr;
	}
	
	WindowPosition Window::GetPosition() const
	{
		RECT rect;
		::GetWindowRect(m_handle, &rect);
		return { static_cast<i32>(rect.left), static_cast<i32>(rect.top) };
	}
	
	WindowSize Window::GetSize() const
	{
		return m_size;
	}
	
	WindowHandle Window::GetHandle() const
	{
		return m_handle;
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
	
	void Window::SetPosition(const WindowPosition& position) const
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
		m_minimumSize = { std::max(size.Width, m_minimumSize.Width), std::max(size.Height, m_minimumSize.Height) };
	}
		
	void Window::SetTitle(const String& title) const
	{
		::SetWindowTextW(m_handle, StringToWString(title).c_str());
	}
	
	void Window::SetVisible(bool visible)
	{
		::ShowWindow(m_handle, visible ? SW_SHOW : SW_HIDE);
	}
	
	void Window::RequestFocus() const
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
	
	void Window::RegisterRawInputDevices() const
	{
		RAWINPUTDEVICE rawMouse = {};
		rawMouse.usUsagePage = 0x01;      // Generic Desktop Controls
		rawMouse.usUsage = 0x02;          // Mouse
		rawMouse.dwFlags = 0;             // No flags
		rawMouse.hwndTarget = m_handle;   // Target our window

		::RegisterRawInputDevices(&rawMouse, 1, sizeof(RAWINPUTDEVICE));
	}

	void Window::SetDPIAwareness() const
	{
		SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
	}

	void Window::ProcessEvent(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (!m_handle)
		{
			return;
		}

		switch (msg)
		{
		case WM_CLOSE:
		{
			PushEvent(Event::Closed{});
			break;
		}

		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				const u32 width = static_cast<u32>(LOWORD(lParam));
				const u32 height = static_cast<u32>(HIWORD(lParam));
				if (m_size.Width != width || m_size.Height != height)
				{
					m_size = { width, height };
					PushEvent(Event::Resized{ width, height });
				}
			}
			break;
		}

		case WM_SETFOCUS:
		{
			PushEvent(Event::FocusGained{});
			break;
		}

		case WM_KILLFOCUS:
		{
			PushEvent(Event::FocusLost{});
			break;
		}

		case WM_CHAR:
		{
			if (m_keyboard->IsKeyRepeatEnabled() || ((lParam & (1 << 30)) == 0))
			{
				// Get the Unicode character
				auto character = static_cast<char32_t>(wParam);

				// Check for surrogate pairs
				if ((character >= 0xD800) && (character <= 0xDBFF))
				{
					// First part of surrogate pair, save it
					m_surrogate = static_cast<char16_t>(character);
				}
				else
				{
					if ((character >= 0xDC00) && (character <= 0xDFFF))
					{
						// Convert surrogate pair to UTF-32
						char32_t utf32;
						const char16_t utf16[] = { m_surrogate, static_cast<char16_t>(character) };
						if (MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf16), 4,
							reinterpret_cast<LPWSTR>(&utf32), 4))
						{
							PushEvent(Event::TextInput{ utf32 });
						}
						m_surrogate = 0;
					}
					else
					{
						// Single character
						PushEvent(Event::TextInput{ character });
					}
				}
			}
			break;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (m_keyboard->IsKeyRepeatEnabled() || ((HIWORD(lParam) & KF_REPEAT) == 0))
			{
				Event::KeyPressed event;
				event.Key     = Keyboard::ToKeyCode(static_cast<i32>(wParam));
				event.Alt     = HIWORD(GetKeyState(VK_MENU)) != 0;
				event.Control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
				event.Shift   = HIWORD(GetKeyState(VK_SHIFT)) != 0;
				event.System  = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
				PushEvent(event);
			}
			break;
		}

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			if (m_keyboard->IsKeyRepeatEnabled() || ((HIWORD(lParam) & KF_REPEAT) == 0))
			{
				Event::KeyReleased event;
				event.Key     = Keyboard::ToKeyCode(static_cast<i32>(wParam));
				event.Alt     = HIWORD(GetKeyState(VK_MENU)) != 0;
				event.Control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
				event.Shift   = HIWORD(GetKeyState(VK_SHIFT)) != 0;
				event.System  = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
				PushEvent(event);
			}
			break;
			break;
		}

		case WM_MOUSEWHEEL:
		{
			// Mouse position is in screen coordinates, convert to window coordinates
			POINT position;
			position.x = static_cast<SHORT>(LOWORD(lParam));
			position.y = static_cast<SHORT>(HIWORD(lParam));
			ScreenToClient(m_handle, &position);

			auto delta = static_cast<SHORT>(HIWORD(wParam));
			PushEvent(Event::MouseWheelScrolled{
				KeyCode::MouseWheel::Vertical,
				static_cast<f32>(delta) / WHEEL_DELTA,
				position.x,
				position.y
				});

			break;
		}

		case WM_MOUSEHWHEEL:
		{
			POINT position;
			position.x = static_cast<SHORT>(LOWORD(lParam));
			position.y = static_cast<SHORT>(HIWORD(lParam));
			ScreenToClient(m_handle, &position);

			auto delta = static_cast<SHORT>(HIWORD(wParam));
			PushEvent(Event::MouseWheelScrolled{
				KeyCode::MouseWheel::Horizontal,
				static_cast<f32>(delta) / WHEEL_DELTA,
				position.x,
				position.y
				});

			break;
		}

		case WM_LBUTTONDOWN:
		{
			PushEvent(Event::MouseButtonPressed{
				KeyCode::MouseButton::Left,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}
		case WM_LBUTTONUP:
		{
			PushEvent(Event::MouseButtonReleased{
				KeyCode::MouseButton::Left,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}
		case WM_RBUTTONDOWN:
		{
			PushEvent(Event::MouseButtonPressed{
				KeyCode::MouseButton::Right,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}
		case WM_RBUTTONUP:
		{
			PushEvent(Event::MouseButtonReleased{
				KeyCode::MouseButton::Right,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}
		case WM_MBUTTONDOWN:
		{
			PushEvent(Event::MouseButtonPressed{
				KeyCode::MouseButton::Middle,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}
		case WM_MBUTTONUP:
		{
			PushEvent(Event::MouseButtonReleased{
				KeyCode::MouseButton::Middle,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}
		case WM_XBUTTONDOWN:
		{
			PushEvent(Event::MouseButtonPressed{
				HIWORD(wParam) == XBUTTON1 ? KeyCode::MouseButton::Extra1 : KeyCode::MouseButton::Extra2,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}
		case WM_XBUTTONUP:
		{
			PushEvent(Event::MouseButtonReleased{
				HIWORD(wParam) == XBUTTON1 ? KeyCode::MouseButton::Extra1 : KeyCode::MouseButton::Extra2,
				static_cast<i32>(LOWORD(lParam)),
				static_cast<i32>(HIWORD(lParam))
				});
			break;
		}

		case WM_MOUSEMOVE:
		{
			const int x = static_cast<SHORT>(LOWORD(lParam));
			const int y = static_cast<SHORT>(HIWORD(lParam));

			// Get the client area of the window
			RECT area;
			GetClientRect(m_handle, &area);

			// Capture the mouse if any button is pressed
			if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) == 0)
			{
				if (::GetCapture() == m_handle)
					::ReleaseCapture();
			}
			else if (::GetCapture() != m_handle)
			{
				::SetCapture(m_handle);
			}

			// Generate MouseEntered/MouseLeft events
			if ((x < area.left) || (x > area.right) || (y < area.top) || (y > area.bottom))
			{
				if (m_mouse->IsInside())
				{
					m_mouse->m_isInside = false;
					PushEvent(Event::MouseLeft{});
				}
			}
			else
			{
				if (!m_mouse->IsInside())
				{
					m_mouse->m_isInside = true;
					PushEvent(Event::MouseEntered{});
				}
			}

			PushEvent(Event::MouseMoved{ x, y });
			break;
		}
		case WM_INPUT:
		{
			RAWINPUT input;
			UINT size = sizeof(input);

			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) != static_cast<UINT>(-1))
			{
				if (input.header.dwType == RIM_TYPEMOUSE && (input.data.mouse.usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
				{
					PushEvent(Event::MouseMovedRaw{ input.data.mouse.lLastX, input.data.mouse.lLastY });
				}
			}
			break;
		}

		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = m_minimumSize.Width;
			lpMMI->ptMinTrackSize.y = m_minimumSize.Height;
		}
		}
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