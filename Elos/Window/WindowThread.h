#pragma once

#include <Elos/Export.h>
#include <Elos/Window/Window.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <any>
#include <future>
#include <atomic>
#include <Windows.h>
#include <Objbase.h>

namespace Elos
{
    class ELOS_API WindowThread
    {
    public:
        WindowThread(Window* window);
        ~WindowThread();

        void QueueCommand(Window::CommandType type, std::any data);
        void QueueCommandAndWait(Window::CommandType type, std::any data);
        HWND GetHandle() const { return m_handle; }
        void SetHandle(HWND handle) { m_handle = handle; }
        LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        void ThreadMain();

        void ProcessCommands();
        void ProcessCommand(const Window::Command& cmd);

        void CreateWindowOnThread(const WindowCreateInfo& info);
        DWORD GetWin32WindowStyle(WindowStyle style, WindowChildMode childMode) const;
        WindowSize ContentSizeToWindowSize(const WindowSize& size) const;

        static LRESULT CALLBACK GlobalWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        Window*                     m_window;
        std::thread                 m_thread;
        std::atomic<bool>           m_running{ true };
        std::queue<Window::Command> m_commandQueue;
        std::mutex                  m_commandMutex;
        std::condition_variable     m_commandCV;
        HWND                        m_handle = nullptr;
    };

    inline WindowThread::WindowThread(Window* window)
        : m_window(window)
    {
        // Start the window thread
        m_thread = std::thread(&WindowThread::ThreadMain, this);
    }

    inline WindowThread::~WindowThread()
    {
        // Signal thread to stop
        m_running = false;

        // Wake up thread
        {
            std::unique_lock<std::mutex> lock(m_commandMutex);
            m_commandCV.notify_all();
        }

        // Join thread
        if (m_thread.joinable())
            m_thread.join();
    }

    inline void WindowThread::QueueCommand(Window::CommandType type, std::any data)
    {
        Window::Command cmd{ type, data };

        {
            std::unique_lock<std::mutex> lock(m_commandMutex);
            m_commandQueue.push(cmd);
        }

        m_commandCV.notify_one();
    }

    inline void WindowThread::QueueCommandAndWait(Window::CommandType type, std::any data)
    {
        std::promise<void> promise;
        std::future<void> future = promise.get_future();

        Window::Command cmd{ type, data, &promise };

        {
            std::unique_lock<std::mutex> lock(m_commandMutex);
            m_commandQueue.push(cmd);
        }

        m_commandCV.notify_one();
        future.wait(); // Wait for command to complete
    }

    inline void WindowThread::ThreadMain()
    {
        // Initialize COM for this thread
        std::ignore = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        // Create message loop
        MSG msg{};

        while (m_running)
        {
            // Process commands from the main thread
            ProcessCommands();

            // Process window messages
            while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_running = false;
                    break;
                }

                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }

            // Don't burn CPU
            if (m_commandQueue.empty())
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Cleanup COM
        CoUninitialize();
    }

    inline void WindowThread::ProcessCommands()
    {
        std::unique_lock<std::mutex> lock(m_commandMutex);

        // Process all commands
        while (!m_commandQueue.empty())
        {
            Window::Command cmd = m_commandQueue.front();
            m_commandQueue.pop();

            // Release lock during command processing
            lock.unlock();
            ProcessCommand(cmd);
            lock.lock();
        }
    }

    inline void WindowThread::ProcessCommand(const Window::Command& cmd)
    {
        try
        {
            switch (cmd.type)
            {
            case Window::CommandType::Create:
                CreateWindowOnThread(std::any_cast<WindowCreateInfo>(cmd.data));
                break;

            case Window::CommandType::Close:
                if (m_handle)
                {
                    ::DestroyWindow(m_handle);
                    m_handle = nullptr;
                    m_window->m_handle = nullptr;
                }
                break;

            case Window::CommandType::SetPosition:
                if (m_handle)
                {
                    auto pos = std::any_cast<WindowPosition>(cmd.data);
                    ::SetWindowPos(m_handle, nullptr, pos.X, pos.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                }
                break;

            case Window::CommandType::SetSize:
                if (m_handle)
                {
                    auto size = std::any_cast<WindowSize>(cmd.data);
                    auto winSize = ContentSizeToWindowSize(size);
                    ::SetWindowPos(m_handle, nullptr, 0, 0, winSize.Width, winSize.Height, SWP_NOMOVE | SWP_NOZORDER);

                    // Update the window's size
                    m_window->m_size = size;
                }
                break;

            case Window::CommandType::SetTitle:
                if (m_handle)
                {
                    auto title = std::any_cast<String>(cmd.data);
                    ::SetWindowTextW(m_handle, StringToWString(title).c_str());
                    m_window->m_title = title;
                }
                break;

            case Window::CommandType::SetVisible:
                if (m_handle)
                {
                    auto visible = std::any_cast<bool>(cmd.data);
                    ::ShowWindow(m_handle, visible ? SW_SHOW : SW_HIDE);
                }
                break;

            case Window::CommandType::RequestFocus:
                if (m_handle)
                {
                    ::SetForegroundWindow(m_handle);
                    ::SetFocus(m_handle);
                }
                break;

            case Window::CommandType::Redraw:
                if (m_handle)
                {
                    ::InvalidateRect(m_handle, nullptr, TRUE);
                    ::UpdateWindow(m_handle);
                }
                break;
            }
        }
        catch (const std::exception& e)
        {
            throw e;
        }

        // Signal completion
        if (cmd.completion)
            cmd.completion->set_value();
    }

    inline void WindowThread::CreateWindowOnThread(const WindowCreateInfo& createInfo)
    {
        static bool classRegistered = false;

        if (!classRegistered)
        {
            WNDCLASSW windowClass     = { 0 };
            windowClass.style         = CS_HREDRAW | CS_VREDRAW;
            windowClass.lpfnWndProc   = &WindowThread::GlobalWndProc;
            windowClass.cbClsExtra    = 0;
            windowClass.cbWndExtra    = 0;
            windowClass.hInstance     = GetModuleHandle(nullptr);
            windowClass.hIcon         = nullptr;
            windowClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
            windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            windowClass.lpszMenuName  = nullptr;
            windowClass.lpszClassName = m_window->s_className;

            if (::RegisterClassW(&windowClass))
                classRegistered = true;
        }

        m_window->SetDPIAwareness();

        // Get parent window handle (if any)
        HWND parentHandle = nullptr;
        if (auto parent = m_window->m_parent.lock())
        {
            parentHandle = parent->GetHandle();
        }

        i32 x = createInfo.Position.X;
        i32 y = createInfo.Position.Y;

        if (x == CW_USEDEFAULT && y == CW_USEDEFAULT)
        {
            if (auto parent = m_window->m_parent.lock())
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

        DWORD win32Style             = GetWin32WindowStyle(createInfo.Style, createInfo.ChildMode);
        const WindowSize& windowSize = ContentSizeToWindowSize(createInfo.Size);

        m_handle = ::CreateWindowEx(
            WS_EX_CLIENTEDGE,
            m_window->s_className,
            StringToWString(createInfo.Title).c_str(),
            win32Style,
            x, y,
            windowSize.Width, windowSize.Height,
            parentHandle,
            nullptr,
            ::GetModuleHandle(nullptr),
            this);

        if (m_handle)
        {
            m_window->m_handle = m_handle;

            // Store the window pointer
            ::SetWindowLongPtr(m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

            // Setup raw input devices
            RAWINPUTDEVICE rawMouse = {};
            rawMouse.usUsagePage    = 0x01;
            rawMouse.usUsage        = 0x02;
            rawMouse.dwFlags        = 0;
            rawMouse.hwndTarget     = m_handle;
            ::RegisterRawInputDevices(&rawMouse, 1, sizeof(RAWINPUTDEVICE));

            ++m_window->s_windowCount;

            ::ShowWindow(m_handle, SW_SHOW);

            m_window->m_size = createInfo.Size;
        }
    }

    inline DWORD WindowThread::GetWin32WindowStyle(WindowStyle style, WindowChildMode childMode) const
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
            win32Style &= ~(WS_BORDER | WS_DLGFRAME);
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

    inline WindowSize WindowThread::ContentSizeToWindowSize(const WindowSize& size) const
    {
        RECT rectangle = { 0, 0, static_cast<LONG>(size.Width), static_cast<LONG>(size.Height) };

        if (m_handle)
        {
            ::AdjustWindowRect(&rectangle, static_cast<DWORD>(GetWindowLongPtr(m_handle, GWL_STYLE)), false);
        }
        else
        {
            // Default adjustment
            rectangle.right += 16;
            rectangle.bottom += 39;
        }

        return { static_cast<u32>(rectangle.right - rectangle.left), static_cast<u32>(rectangle.bottom - rectangle.top) };
    }

    inline LRESULT WindowThread::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Special handling for painting to avoid mutex deadlocks
        if (msg == WM_PAINT || msg == WM_ERASEBKGND)
        {
            if (msg == WM_PAINT)
            {
                PAINTSTRUCT ps;
                HDC hdc = ::BeginPaint(m_handle, &ps);

                RECT rect;
                ::GetClientRect(m_handle, &rect);

                HBRUSH brush = ::CreateSolidBrush(m_window->m_backgroundColor);
                HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);

                ::FillRect(hdc, &ps.rcPaint, brush);

                ::SelectObject(hdc, oldBrush);
                ::DeleteObject(brush);

                ::EndPaint(m_handle, &ps);
                return 0;
            }
            else if (msg == WM_ERASEBKGND)
            {
                HDC hdc = (HDC)wParam;
                RECT rect;
                ::GetClientRect(m_handle, &rect);

                HBRUSH brush = ::CreateSolidBrush(m_window->m_backgroundColor);
                HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);

                ::FillRect(hdc, &rect, brush);

                ::SelectObject(hdc, oldBrush);
                ::DeleteObject(brush);

                return TRUE;
            }
        }

        // Process all other messages and generate events
        switch (msg)
        {
        case WM_CLOSE:
            m_window->PushEvent(Event::Closed{});
            return 0;

        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                const u32 width = static_cast<u32>(LOWORD(lParam));
                const u32 height = static_cast<u32>(HIWORD(lParam));

                if (m_window->m_size.Width != width || m_window->m_size.Height != height)
                {
                    m_window->m_size = { width, height };
                    m_window->PushEvent(Event::Resized{ width, height });
                }
            }
            break;

        case WM_SETFOCUS:
            m_window->PushEvent(Event::FocusGained{});
            break;

        case WM_KILLFOCUS:
            m_window->PushEvent(Event::FocusLost{});
            break;

        case WM_CHAR:
            if (m_window->m_keyboard->IsKeyRepeatEnabled() || ((lParam & (1 << 30)) == 0))
            {
                // Get the Unicode character
                auto character = static_cast<char32_t>(wParam);

                // Check for surrogate pairs
                if ((character >= 0xD800) && (character <= 0xDBFF))
                {
                    // First part of surrogate pair, save it
                    m_window->m_surrogate = static_cast<char16_t>(character);
                }
                else
                {
                    if ((character >= 0xDC00) && (character <= 0xDFFF))
                    {
                        // Convert surrogate pair to UTF-32
                        char32_t utf32;
                        const char16_t utf16[] = { m_window->m_surrogate, static_cast<char16_t>(character) };
                        if (MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(utf16), 4,
                            reinterpret_cast<LPWSTR>(&utf32), 4))
                        {
                            m_window->PushEvent(Event::TextInput{ utf32 });
                        }
                        m_window->m_surrogate = 0;
                    }
                    else
                    {
                        // Single character
                        m_window->PushEvent(Event::TextInput{ character });
                    }
                }
            }
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (m_window->m_keyboard->IsKeyRepeatEnabled() || ((HIWORD(lParam) & KF_REPEAT) == 0))
            {
                Event::KeyPressed event;
                event.Key     = Keyboard::ToKeyCode(static_cast<i32>(wParam));
                event.Alt     = HIWORD(GetKeyState(VK_MENU)) != 0;
                event.Control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                event.Shift   = HIWORD(GetKeyState(VK_SHIFT)) != 0;
                event.System  = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                m_window->PushEvent(event);
            }
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (m_window->m_keyboard->IsKeyRepeatEnabled() || ((HIWORD(lParam) & KF_REPEAT) == 0))
            {
                Event::KeyReleased event;
                event.Key     = Keyboard::ToKeyCode(static_cast<i32>(wParam));
                event.Alt     = HIWORD(GetKeyState(VK_MENU)) != 0;
                event.Control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                event.Shift   = HIWORD(GetKeyState(VK_SHIFT)) != 0;
                event.System  = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                m_window->PushEvent(event);
            }
            break;

        case WM_MOUSEWHEEL:
        {
            // Mouse position is in screen coordinates, convert to window coordinates
            POINT position;
            position.x = static_cast<SHORT>(LOWORD(lParam));
            position.y = static_cast<SHORT>(HIWORD(lParam));
            ScreenToClient(m_handle, &position);

            auto delta = static_cast<SHORT>(HIWORD(wParam));
            m_window->PushEvent(Event::MouseWheelScrolled
            {
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
            m_window->PushEvent(Event::MouseWheelScrolled
            {
                KeyCode::MouseWheel::Horizontal,
                static_cast<f32>(delta) / WHEEL_DELTA,
                position.x,
                position.y
            });
            break;
        }

        case WM_LBUTTONDOWN:
        {
            m_window->PushEvent(Event::MouseButtonPressed
            {
                KeyCode::MouseButton::Left,
                static_cast<i32>(LOWORD(lParam)),
                static_cast<i32>(HIWORD(lParam))
            });
            break;
        }

        case WM_LBUTTONUP:
        {
            m_window->PushEvent(Event::MouseButtonReleased
            {
                KeyCode::MouseButton::Left,
                static_cast<i32>(LOWORD(lParam)),
                static_cast<i32>(HIWORD(lParam))
            });
            break;
        }

        case WM_RBUTTONDOWN:
        {
            m_window->PushEvent(Event::MouseButtonPressed
            {
                KeyCode::MouseButton::Right,
                static_cast<i32>(LOWORD(lParam)),
                static_cast<i32>(HIWORD(lParam))
            });
            break;
        }

        case WM_RBUTTONUP:
        {
            m_window->PushEvent(Event::MouseButtonReleased
            {
                KeyCode::MouseButton::Right,
                static_cast<i32>(LOWORD(lParam)),
                static_cast<i32>(HIWORD(lParam))
            });
            break;
        }

        case WM_MBUTTONDOWN:
        {
            m_window->PushEvent(Event::MouseButtonPressed
            {
                KeyCode::MouseButton::Middle,
                static_cast<i32>(LOWORD(lParam)),
                static_cast<i32>(HIWORD(lParam))
            });
            break;
        }

        case WM_MBUTTONUP:
        {
            m_window->PushEvent(Event::MouseButtonReleased
            {
                KeyCode::MouseButton::Middle,
                static_cast<i32>(LOWORD(lParam)),
                static_cast<i32>(HIWORD(lParam))
            });
            break;
        }

        case WM_XBUTTONDOWN:
        {
            m_window->PushEvent(Event::MouseButtonPressed
            {
                HIWORD(wParam) == XBUTTON1 ? KeyCode::MouseButton::Extra1 : KeyCode::MouseButton::Extra2,
                static_cast<i32>(LOWORD(lParam)),
                static_cast<i32>(HIWORD(lParam))
            });
            break;
        }

        case WM_XBUTTONUP:
        {
            m_window->PushEvent(Event::MouseButtonReleased
            {
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
            ::GetClientRect(m_handle, &area);

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
                if (m_window->m_mouse->IsInside())
                {
                    m_window->m_mouse->m_isInside = false;
                    m_window->PushEvent(Event::MouseLeft{});
                }
            }
            else
            {
                if (!m_window->m_mouse->IsInside())
                {
                    m_window->m_mouse->m_isInside = true;
                    m_window->PushEvent(Event::MouseEntered{});
                }
            }

            m_window->PushEvent(Event::MouseMoved{ x, y });
            break;
        }

        case WM_INPUT:
        {
            RAWINPUT input;
            UINT size = sizeof(input);

            if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) != static_cast<UINT>(-1))
            {
                if (input.header.dwType == RIM_TYPEMOUSE && (input.data.mouse.usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
                {
                    m_window->PushEvent(Event::MouseMovedRaw{ input.data.mouse.lLastX, input.data.mouse.lLastY });
                }
            }
            break;
        }

        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = m_window->m_minimumSize.Width;
            lpMMI->ptMinTrackSize.y = m_window->m_minimumSize.Height;
        }
            break;
        }

        return ::DefWindowProc(m_handle, msg, wParam, lParam);
    }

    inline LRESULT CALLBACK WindowThread::GlobalWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // Get the window thread from window data
        WindowThread* threadContext = reinterpret_cast<WindowThread*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

        if (threadContext)
            return threadContext->ProcessMessage(message, wParam, lParam);

        return ::DefWindowProc(hwnd, message, wParam, lParam);
    }
}