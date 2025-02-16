#include <Elos/Window/Window.h>

#include <Elos/Common/Assert.h>
#include <shellscalingapi.h>
#include <CommCtrl.h>

namespace Elos
{	
	u32 Window::s_windowCount = 0;
	const wchar_t* Window::s_className = L"ElosWindowClass";

	Window::Window(const WindowCreateInfo& createInfo)
	{
		// Init common controls
		if (s_windowCount == 0)
		{
			INITCOMMONCONTROLSEX icc = {};
			icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
			InitCommonControlsEx(&icc);
		}


		m_keyboard = std::make_unique<Keyboard>(*this);
		m_mouse = std::make_unique<Mouse>(*this);
		Create(createInfo);
	}
	
	Window::~Window()
	{
		// Close the handle if we are still open
		if (m_handle)
		{
			Close();
		}

		m_keyboard.reset();
		m_mouse.reset();
	}
	
	std::shared_ptr<Window> Window::CreateChild(const WindowCreateInfo& createInfo)
	{
		WindowCreateInfo info(createInfo);

		if (!createInfo.Parent)
		{
			info.Parent = shared_from_this();
		}

		return AddChild(info);
	}

	void Window::Create(const WindowCreateInfo& createInfo)
	{
		RegisterWindowClass();
		SetDPIAwareness();

		m_title     = createInfo.Title;
		m_childMode = createInfo.ChildMode;
		m_parent    = createInfo.Parent;

		auto parent = m_parent.lock();

		// Compute position
		i32 x = createInfo.Position.X;
		i32 y = createInfo.Position.Y;

		if (x == CW_USEDEFAULT && y == CW_USEDEFAULT)
		{
			if (parent)
			{
				// Center in parent
				const WindowSize& parentSize = parent->GetSize();
				x = (parentSize.Width - createInfo.Size.Width) / 2;
				y = (parentSize.Height - createInfo.Size.Height) / 2;
			}
			else
			{
				// Center on screen
				const HDC screenDC = ::GetDC(nullptr);
				x = ::GetDeviceCaps(screenDC, HORZRES) / 2 - static_cast<i32>(createInfo.Size.Width) / 2;
				y = ::GetDeviceCaps(screenDC, VERTRES) / 2 - static_cast<i32>(createInfo.Size.Height) / 2;
				::ReleaseDC(nullptr, screenDC);
			}
		}

		DWORD win32Style = GetWin32WindowStyle(createInfo.Style, createInfo.ChildMode);
		const WindowSize& windowSize = ContentSizeToWindowSize(createInfo.Size);

		// Get parent handle if this is a child window
		HWND parentHandle = parent ? parent->GetHandle() : nullptr;

		m_handle = ::CreateWindowEx(
			WS_EX_CLIENTEDGE,
			s_className,
			StringToWString(createInfo.Title).c_str(),
			win32Style,
			x, y,
			windowSize.Width, windowSize.Height,
			parentHandle,
			nullptr,
			::GetModuleHandle(nullptr),
			this);

		if (!m_handle)
		{
#if ELOS_BUILD_DEBUG
			ASSERT(m_handle)
				.Msg("Failed to create a window [{}] (GetLastError={})", m_title, ::GetLastError())
				.Throw();
#endif
			return;
		}

		::SetWindowLongPtrW(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		

		RegisterRawInputDevices();
		SetVisible(true);

		// Only set focus if we're not a child or we're a modal window
		if (!parent || m_childMode == WindowChildMode::Modal)
		{
			::SetForegroundWindow(m_handle);
			::SetFocus(m_handle);
		}

		// Calculate initial relative positions
		if (m_childMode == WindowChildMode::Embedded && parent)
		{
			const WindowSize& parentSize  = parent->GetSize();
			m_relativeRect.RelativeWidth  = static_cast<f32>(createInfo.Size.Width) / parentSize.Width;
			m_relativeRect.RelativeHeight = static_cast<f32>(createInfo.Size.Height) / parentSize.Height;
		}

		m_size = windowSize;
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
		// If the handle for this window is null that means its already been closed
		if (m_handle)
		{
			for (auto& child : m_children)
			{
				if (child)
				{
					child->Close();
				}
			}
			m_children.clear();

			if (auto parent = m_parent.lock(); parent.get())
			{
				if (parent->GetHandle())
				{
					parent->RemoveChild(shared_from_this());
				}
			}

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
		m_title = title;
	}
	
	void Window::SetVisible(bool visible) const
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
		PumpMessages();

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
		// Check if the class is already registered
		WNDCLASSW wc;
		if (::GetClassInfoW(GetModuleHandleW(nullptr), s_className, &wc))
		{
			return;
		}

		WNDCLASSW windowClass     = { 0 };
		windowClass.style         = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc   = &Window::GlobalOnEvent;
		windowClass.cbClsExtra    = 0;
		windowClass.cbWndExtra    = 0;
		windowClass.hInstance     = GetModuleHandle(nullptr);
		windowClass.hIcon         = nullptr;
		windowClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		windowClass.lpszMenuName  = nullptr;
		windowClass.lpszClassName = s_className;

		BOOL result = ::RegisterClassW(&windowClass);

#if ELOS_BUILD_DEBUG
		ASSERT(result)
			.Msg("Failed to register window class. (GetLastError={})", ::GetLastError())
			.Throw();
#endif
	}
	
	LRESULT Window::GlobalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* window = handle ? reinterpret_cast<Window*>(::GetWindowLongPtr(handle, GWLP_USERDATA)) : nullptr;

		if (window)
		{
			if (LRESULT result = window->ProcessEvent(message, wParam, lParam))
			{
				return result;  // Event has been handled by window
			}
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

#if ELOS_BUILD_DEBUG
		ASSERT_NOT_NULL(handle).Throw();
#endif

		return handle ? ::DefWindowProc(handle, message, wParam, lParam) : 0;
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

	LRESULT Window::ProcessEvent(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (!m_handle)
		{
			return 0;
		}

		switch (msg)
		{
		case WM_ERASEBKGND:
		{
			// Notify the OS that we don't want to erase the background to prevent flicker
			HDC hdc = (HDC)wParam;
			RECT rect;
			::GetClientRect(m_handle, &rect);

			// Create and select a brush with the background color
			HBRUSH brush = ::CreateSolidBrush(m_backgroundColor);
			HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);

			// Fill the entire client area
			::FillRect(hdc, &rect, brush);

			// Clean up
			::SelectObject(hdc, oldBrush);
			::DeleteObject(brush);

			return TRUE;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(m_handle, &ps);

			// Get client area
			RECT rect;
			::GetClientRect(m_handle, &rect);

			// Create and select a brush with the background color
			HBRUSH brush = ::CreateSolidBrush(m_backgroundColor);
			HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);

			// Fill the entire client area
			::FillRect(hdc, &ps.rcPaint, brush);

			// Clean up
			::SelectObject(hdc, oldBrush);
			::DeleteObject(brush);

			::EndPaint(m_handle, &ps);
			return 0;
		}

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

					// Update embedded children
					for (auto& child : m_children)
					{
						if (child && child->GetChildMode() == WindowChildMode::Embedded)
						{
							const auto& rel = child->m_relativeRect;
							const WindowSize newSize
							{
								static_cast<u32>(width * rel.RelativeWidth),
								static_cast<u32>(height * rel.RelativeHeight)
							};

							child->SetSize(newSize);
						}
					}

					PushEvent(Event::Resized{ width, height });
				}
			}
			break;
		}

		case WM_SIZING:
		{
			if (m_childMode == WindowChildMode::Embedded)
			{
				RECT* rect = reinterpret_cast<RECT*>(lParam);
				if (auto parent = m_parent.lock())
				{
					// Get parent client area
					RECT parentRect;
					::GetClientRect(parent->GetHandle(), &parentRect);
					::MapWindowPoints(parent->GetHandle(), HWND_DESKTOP, reinterpret_cast<POINT*>(&parentRect), 2);

					// Constrain size to parent window
					if (rect->right > parentRect.right)
						rect->right = parentRect.right;
					if (rect->bottom > parentRect.bottom)
						rect->bottom = parentRect.bottom;

					// Update relative sizes
					const WindowSize& parentSize  = parent->GetSize();
					m_relativeRect.RelativeWidth  = static_cast<f32>(rect->right - rect->left) / parentSize.Width;
					m_relativeRect.RelativeHeight = static_cast<f32>(rect->bottom - rect->top) / parentSize.Height;
				}
				return 1;
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
			const i32 x = static_cast<i32>(LOWORD(lParam));
			const i32 y = static_cast<i32>(HIWORD(lParam));

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

		return 0;
	}
	
	void Window::PushEvent(const Event& event)
	{
		m_events.push(event);
	}
	
	void Window::PumpMessages()
	{
		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
	}

	void Window::Redraw() const
	{
		::InvalidateRect(m_handle, nullptr, TRUE);
		::UpdateWindow(m_handle);
	}
	
	WindowSize Window::ContentSizeToWindowSize(const WindowSize& size) const
	{
		RECT rectangle = { 0, 0, static_cast<LONG>(size.Width), static_cast<LONG>(size.Height) };
		::AdjustWindowRect(&rectangle, static_cast<DWORD>(GetWindowLongPtr(m_handle, GWL_STYLE)), false);
		return { static_cast<u32>(rectangle.right - rectangle.left), static_cast<u32>(rectangle.bottom - rectangle.top) };
	}
	
	std::shared_ptr<Window> Window::AddChild(const WindowCreateInfo& createInfo)
	{
		std::shared_ptr<Window> child = std::make_shared<Window>(createInfo);
		m_children.push_back(child);
		return child;
	}
	
	void Window::RemoveChild(std::shared_ptr<Window> child)
	{
		m_children.erase(std::remove(
			m_children.begin(), m_children.end(), child),
		m_children.end());
	}
	
	DWORD Window::GetWin32WindowStyle(WindowStyle style, WindowChildMode childMode) const
	{
		DWORD win32Style = 0;

		switch (childMode)
		{
		case WindowChildMode::None:
			win32Style = WS_VISIBLE | WS_CLIPCHILDREN;
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
			break;

		case WindowChildMode::Embedded:
			win32Style = WS_CHILD | WS_VISIBLE | WS_THICKFRAME | WS_CLIPCHILDREN;
			win32Style &= ~(WS_BORDER | WS_DLGFRAME);  // Remove the default window border
			break;

		case WindowChildMode::Modal:
			win32Style = WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU;
			if ((style & WindowStyle::Resize) == WindowStyle::Resize)
				win32Style |= WS_THICKFRAME;
			break;

		case WindowChildMode::Popup:
			win32Style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
			if ((style & WindowStyle::Resize) == WindowStyle::Resize)
				win32Style |= WS_THICKFRAME;
			break;
		}

		return win32Style;
	}
}