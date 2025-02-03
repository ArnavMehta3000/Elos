#pragma once

#include <Elos/Export.h>
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/FunctionMacros.h>
#include <Elos/Window/Input/KeyCode.h>

namespace Elos
{
	class Window;

	class ELOS_API Keyboard
	{
		friend class Window;

	public:
		explicit Keyboard(Window& window);

		NODISCARD bool IsKeyRepeatEnabled() const noexcept;
		NODISCARD bool IsKeyPressed(KeyCode::Key key) const noexcept;

		void SetKeyRepeatEnabled(bool enabled);

	private:
		Window& m_window;
		bool m_isKeyRepeatEnabled;

		static i32 ToVirtualKey(KeyCode::Key key);
		static KeyCode::Key ToKeyCode(i32 key);
	};
}