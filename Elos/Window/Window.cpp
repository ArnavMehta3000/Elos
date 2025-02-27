#include <Elos/Window/Window.h>
#include <Elos/Window/WindowThread.h>
#include <ShellScalingApi.h>

namespace Elos
{
    u32 Window::s_windowCount = 0;
    const wchar_t* Window::s_className = L"ElosWindowClass";

    Window::Window(const WindowCreateInfo& createInfo)
    {
        m_keyboard = std::make_unique<Keyboard>(*this);
        m_mouse = std::make_unique<Mouse>(*this);

        m_title     = createInfo.Title;
        m_childMode = createInfo.ChildMode;
        m_parent    = createInfo.Parent;

        m_windowThread = std::make_unique<WindowThread>(this);

        // Create the window on its own thread
        QueueCommandAndWait(CommandType::Create, createInfo);
    }

    Window::~Window()
    {
        // Close the window if needed
        if (IsOpen())
            Close();

        // Stop the window thread
        m_windowThread.reset();
    }

    std::shared_ptr<Window> Window::CreateChild(const WindowCreateInfo& createInfo)
    {
        WindowCreateInfo info(createInfo);

        if (!createInfo.Parent)
            info.Parent = shared_from_this();

        std::shared_ptr<Window> child = std::make_shared<Window>(info);

        // Add to children list
        std::lock_guard<std::recursive_mutex> lock(m_windowMutex);
        m_children.push_back(child);

        return child;
    }

    bool Window::IsOpen() const
    {
        return m_handle != nullptr;
    }

    WindowPosition Window::GetPosition() const
    {
        std::lock_guard<std::recursive_mutex> lock(m_windowMutex);

        if (!m_handle)
            return { 0, 0 };

        RECT rect;
        ::GetWindowRect(m_handle, &rect);
        return { static_cast<i32>(rect.left), static_cast<i32>(rect.top) };
    }

    WindowSize Window::GetSize() const
    {
        std::lock_guard<std::recursive_mutex> lock(m_windowMutex);
        return m_size;
    }

    WindowHandle Window::GetHandle() const
    {
        return m_handle;
    }

    void Window::Close()
    {
        QueueCommandAndWait(CommandType::Close);
    }

    void Window::SetPosition(const WindowPosition& position)
    {
        QueueCommand(CommandType::SetPosition, position);
    }

    void Window::SetSize(const WindowSize& size)
    {
        QueueCommand(CommandType::SetSize, size);
    }

    void Window::SetMinimumSize(const WindowSize& size)
    {
        std::lock_guard<std::recursive_mutex> lock(m_windowMutex);
        m_minimumSize = {
            std::max(size.Width, m_minimumSize.Width),
            std::max(size.Height, m_minimumSize.Height)
        };
    }

    void Window::SetTitle(const String& title)
    {
        QueueCommand(CommandType::SetTitle, title);
    }

    void Window::SetVisible(bool visible)
    {
        QueueCommand(CommandType::SetVisible, visible);
    }

    void Window::RequestFocus()
    {
        QueueCommand(CommandType::RequestFocus);
    }

    bool Window::HasFocus() const
    {
        if (!m_handle)
            return false;

        return m_handle == ::GetForegroundWindow();
    }

    void Window::SetBackgroundColor(COLORREF color)
    {
        std::lock_guard<std::recursive_mutex> lock(m_windowMutex);
        m_backgroundColor = color;

        // Request redraw with new background
        Redraw();
    }

    void Window::Redraw()
    {
        QueueCommand(CommandType::Redraw);
    }

    std::optional<Event> Window::PollEvent()
    {
        return m_events.TryPop();
    }

    void Window::PushEvent(const Event& event)
    {
        m_events.Push(event);
    }

    void Window::QueueCommand(CommandType type, std::any data)
    {
        if (m_windowThread)
            m_windowThread->QueueCommand(type, data);
    }

    void Window::QueueCommandAndWait(CommandType type, std::any data)
    {
        if (m_windowThread)
            m_windowThread->QueueCommandAndWait(type, data);
    }
    
    void Window::SetDPIAwareness() const
    {
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
}