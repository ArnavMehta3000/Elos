#pragma once

#include <Elos/Export.h>
#include <Elos/Common/String.h>
#include <Elos/Containers/ThreadSafeQueue.h>
#include <Elos/Window/Input/Keyboard.h>
#include <Elos/Window/Input/Mouse.h>
#include <Elos/Window/WindowEvents.h>
#include <Elos/Window/WindowHandle.h>
#include <Elos/Window/WindowTypes.h>
#include <memory>
#include <optional>
#include <vector>
#include <any>
#include <Windows.h>
#include <future>

namespace Elos
{
    class WindowThread;

    class ELOS_API Window : public std::enable_shared_from_this<Window>
    {
    public:
        Window() = default;
        Window(const WindowCreateInfo& info);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&& other) noexcept = delete;
        Window& operator=(Window&& other) noexcept = delete;

        NODISCARD std::shared_ptr<Window> CreateChild(const WindowCreateInfo& createInfo);

        NODISCARD bool IsOpen() const;
        NODISCARD WindowPosition GetPosition() const;
        NODISCARD WindowSize GetSize() const;
        NODISCARD WindowHandle GetHandle() const;

        void Close();
        void SetPosition(const WindowPosition& position);
        void SetSize(const WindowSize& size);
        void SetMinimumSize(const WindowSize& size);
        void SetTitle(const String& title);
        void SetVisible(bool visible);
        void RequestFocus();
        bool HasFocus() const;
        void SetBackgroundColor(COLORREF color);
        void Redraw();

        NODISCARD std::optional<Event> PollEvent();

        template <typename... Handlers>
        void HandleEvents(Handlers&&... handlers);

        NODISCARD bool IsChild() const { return m_childMode != WindowChildMode::None; }
        NODISCARD WindowChildMode GetChildMode() const { return m_childMode; }
        NODISCARD std::shared_ptr<Window> GetParent() const { return m_parent.lock(); }
        NODISCARD const std::vector<std::shared_ptr<Window>>& GetChildren() const { return m_children; }

    private:
        friend class WindowThread;

        enum class CommandType
        {
            Create,
            Close,
            SetPosition,
            SetSize,
            SetTitle,
            SetVisible,
            RequestFocus,
            Redraw,
            AddChild,
            RemoveChild
        };

        struct Command
        {
            CommandType type;
            std::any data;
            std::promise<void>* completion = nullptr;
        };

        void QueueCommand(CommandType type, std::any data = {});
        void QueueCommandAndWait(CommandType type, std::any data = {});
        void SetDPIAwareness() const;
        void PushEvent(const Event& event);

    private:
        WindowSize                           m_size{ 0, 0 };
        WindowSize                           m_minimumSize{ 20, 20 };
        char16                               m_surrogate{ 0 };
        WindowChildMode                      m_childMode{ WindowChildMode::None };
        std::unique_ptr<Keyboard>            m_keyboard;
        std::unique_ptr<Mouse>               m_mouse;
        std::weak_ptr<Window>                m_parent;
        ThreadSafeQueue<Event>               m_events;
        std::vector<std::shared_ptr<Window>> m_children;
        mutable String                       m_title;
        COLORREF                             m_backgroundColor = RGB(19, 22, 27);

        // For embedded windows - store position and size as percentages of parent
        struct
        {
            f32 RelativeWidth{ 1.0f };  // Width as a percentage of the parent window
            f32 RelativeHeight{ 1.0f }; // Height as a percentage of the parent window
        } m_relativeRect;

        // Thread-specific data
        std::unique_ptr<WindowThread>        m_windowThread;
        mutable std::recursive_mutex         m_windowMutex;
        WindowHandle                         m_handle{ nullptr };  // Accessed from both threads

        static u32            s_windowCount;
        static const wchar_t* s_className;
    };

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
