#include <Elos/Window/Input/Mouse.h>
#include <Elos/Window/Window.h>
#include <Windows.h>

namespace Elos
{
	Mouse::Mouse(Window& window)
		: m_window(window)
		, m_visible(true)
		, m_locked(false)
		, m_isInside(true)
	{
	}
	
	std::pair<i32, i32> Mouse::GetScreenPosition()
	{
		POINT point;
		::GetCursorPos(&point);
		return std::make_pair(point.x, point.y);
	}
	
	void Mouse::SetScreenPosition(i32 x, i32 y)
	{
		::SetCursorPos(x, y);
	}
	
	std::pair<i32, i32> Mouse::GetPosition() const
	{
		POINT point;
		::GetCursorPos(&point);
		::ScreenToClient(m_window.GetHandle(), &point);
		return std::make_pair(point.x, point.y);
	}
	
	bool Mouse::IsButtonPressed(KeyCode::MouseButton button) const
	{
		i32 virtualKey = ToVirtualKey(button);
		return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
	}
	
	bool Mouse::IsVisible() const
	{
		return m_visible;
	}
	
	bool Mouse::IsLocked() const
	{
		return m_locked;
	}

	NODISCARD bool Mouse::IsInside() const
	{
		return m_isInside;
	}
	
	void Mouse::SetVisible(bool visible)
	{
		m_visible = visible;
		::ShowCursor(visible);
	}
	
	void Mouse::SetPosition(i32 x, i32 y)
	{
		POINT point = { x, y };
		::ClientToScreen(m_window.GetHandle(), &point);
		::SetCursorPos(point.x, point.y);
	}
	
	void Mouse::SetLocked(bool locked)
	{
		m_locked = locked;

		if (m_locked)
		{
			RECT rect;
			::GetClientRect(m_window.GetHandle(), &rect);
			::MapWindowPoints(m_window.GetHandle(), nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
			::ClipCursor(&rect);
		}
		else
		{
			::ClipCursor(nullptr);
		}
	}
	
	i32 Mouse::ToVirtualKey(KeyCode::MouseButton button)
	{
		switch (button)
		{
		case KeyCode::MouseButton::Left:   return ::GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
		case KeyCode::MouseButton::Right:  return ::GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON;
		case KeyCode::MouseButton::Middle: return VK_MBUTTON;
		case KeyCode::MouseButton::Extra1: return VK_XBUTTON1;
		case KeyCode::MouseButton::Extra2: return VK_XBUTTON2;
		default:                           return 0;
		}
	}
}