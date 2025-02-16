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
#include <vector>
#include <Windows.h>

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
        void SetPosition(const WindowPosition& position) const;
        void SetSize(const WindowSize& size);
        void SetMinimumSize(const WindowSize& size);
        void SetTitle(const String& title) const;
        void SetVisible(bool visible) const;
        void RequestFocus() const;
        bool HasFocus() const;
        void SetBackgroundColor(COLORREF color) { m_backgroundColor = color; }
        void Redraw() const;

        std::optional<Event> PollEvent();

        template <typename... Handlers>
        void HandleEvents(Handlers&&... handlers);

        NODISCARD bool IsChild() const { return m_childMode != WindowChildMode::None; }
        NODISCARD WindowChildMode GetChildMode() const { return m_childMode; }
        NODISCARD std::shared_ptr<Window> GetParent() const { return m_parent.lock(); }
        NODISCARD const std::vector<std::shared_ptr<Window>>& GetChildren() const { return m_children; }

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

        NODISCARD std::shared_ptr<Window> AddChild(const WindowCreateInfo& createInfo);
        void RemoveChild(std::shared_ptr<Window> child);
        NODISCARD DWORD GetWin32WindowStyle(WindowStyle style, WindowChildMode childMode) const;

    private:
        WindowHandle                         m_handle{ nullptr };
        WindowSize                           m_size{ 0, 0 };
        WindowSize                           m_minimumSize{ 20, 20 };
        char16                               m_surrogate{ 0 };
        WindowChildMode                      m_childMode{ WindowChildMode::None };
        std::unique_ptr<Keyboard>            m_keyboard;
        std::unique_ptr<Mouse>               m_mouse;
        std::weak_ptr<Window>                m_parent;
        std::queue<Event>                    m_events;
        std::vector<std::shared_ptr<Window>> m_children;
        mutable String                       m_title;
        COLORREF                             m_backgroundColor = RGB(19, 22, 27);
        
        // For embedded windows - store position and size as percentages of parent
        struct
        {
            f32 RelativeWidth{ 1.0f };  // Width as a percentage of the parent window
			f32 RelativeHeight{ 1.0f }; // Height as a percentage of the parent window
        } m_relativeRect;

        static u32 s_windowCount;
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
