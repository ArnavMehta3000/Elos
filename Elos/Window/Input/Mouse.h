#pragma once

#include <Elos/Export.h>
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Window/Input/KeyCode.h>
#include <utility>

namespace Elos
{
	class Window;

	class ELOS_API Mouse
	{
		friend class Window;

	public:
		explicit Mouse(Window& window);

		NODISCARD static std::pair<i32, i32> GetScreenPosition();
		static void SetScreenPosition(i32 x, i32 y);

		NODISCARD std::pair<i32, i32> GetPosition() const;
		NODISCARD bool IsButtonPressed(KeyCode::MouseButton button) const;
		NODISCARD bool IsVisible() const;
		NODISCARD bool IsLocked() const;
		NODISCARD bool IsInside() const;

		void SetVisible(bool visible);
		void SetPosition(i32 x, i32 y);
		void SetLocked(bool locked);
	private:
		Window& m_window;
		bool m_visible;
		bool m_locked;
		bool m_isInside;

		static i32 ToVirtualKey(KeyCode::MouseButton button);
	};
}