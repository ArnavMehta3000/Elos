#pragma once

#include <Elos/Export.h>
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Window/WindowTypes.h>
#include <vector>
#include <memory>

namespace Elos
{
	class Window;

	class ELOS_API Widget
	{
	public:
		Widget() = default;
		virtual ~Widget() = default;

		Widget(const Widget&) = delete;
		Widget& operator=(const Widget&) = delete;

		Widget(Widget&&) = default;
		Widget& operator=(Widget&&) = default;

		virtual void Create(Window& window) = 0;
		virtual void Destroy();
		virtual void Update();
		virtual void Layout();

		void SetParent(Widget* parent) { m_parent = parent; }
		NODISCARD Widget* GetParent() const noexcept{ return m_parent; }
		void AddChild(std::unique_ptr<Widget> child);
		void RemoveChild(Widget* child);

		virtual void SetPosition(const WindowPosition& position) { m_position = position; }
		virtual void SetSize(const WindowSize& size);
		virtual NODISCARD WindowPosition GetPosition() const noexcept { return m_position; }
		virtual NODISCARD WindowSize GetSize() const noexcept { return m_size; }

		virtual void SetVisible(bool visible);
		virtual bool IsVisible() const noexcept { return m_isVisible; }
		virtual void SetEnabled(bool enabled);
		virtual bool IsEnabled() const noexcept { return m_isEnabled; }

		const std::vector<std::unique_ptr<Widget>>& GetChildren() const { return m_children; }
		Window* GetWindow() const noexcept { return m_window; }

	protected:
		Window*                              m_window{ nullptr };
		Widget*                              m_parent{ nullptr };
		WindowPosition                       m_position{ 0,0 };
		WindowSize                           m_size{ 0,0 };
		std::vector<std::unique_ptr<Widget>> m_children;
		bool                                 m_isVisible{ true };
		bool                                 m_isEnabled{ true };
	};

	template<typename T, typename... Args>
	std::unique_ptr<T> CreateWidget(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}