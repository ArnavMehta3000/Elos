#include <Elos/Window/Input/Keyboard.h>
#include <Elos/Window/Window.h>

namespace Elos
{
	Keyboard::Keyboard(Window& window)
		: m_window(window)
		, m_isKeyRepeatEnabled(true)
	{
	}

	bool Keyboard::IsKeyRepeatEnabled() const noexcept
	{
		return m_isKeyRepeatEnabled;
	}

	bool Keyboard::IsKeyPressed(KeyCode::Key key) const noexcept
	{
		i32 virtualKey = ToVirtualKey(key);
		return (::GetAsyncKeyState(virtualKey) & 0x8000) != 0;
	}

	void Keyboard::SetKeyRepeatEnabled(bool enabled)
	{
		m_isKeyRepeatEnabled = enabled;
	}
	
	i32 Keyboard::ToVirtualKey(KeyCode::Key key)
	{
		switch (key)
		{
		case KeyCode::Key::A:          return 'A';
		case KeyCode::Key::B:          return 'B';
		case KeyCode::Key::C:          return 'C';
		case KeyCode::Key::D:          return 'D';
		case KeyCode::Key::E:          return 'E';
		case KeyCode::Key::F:          return 'F';
		case KeyCode::Key::G:          return 'G';
		case KeyCode::Key::H:          return 'H';
		case KeyCode::Key::I:          return 'I';
		case KeyCode::Key::J:          return 'J';
		case KeyCode::Key::K:          return 'K';
		case KeyCode::Key::L:          return 'L';
		case KeyCode::Key::M:          return 'M';
		case KeyCode::Key::N:          return 'N';
		case KeyCode::Key::O:          return 'O';
		case KeyCode::Key::P:          return 'P';
		case KeyCode::Key::Q:          return 'Q';
		case KeyCode::Key::R:          return 'R';
		case KeyCode::Key::S:          return 'S';
		case KeyCode::Key::T:          return 'T';
		case KeyCode::Key::U:          return 'U';
		case KeyCode::Key::V:          return 'V';
		case KeyCode::Key::W:          return 'W';
		case KeyCode::Key::X:          return 'X';
		case KeyCode::Key::Y:          return 'Y';
		case KeyCode::Key::Z:          return 'Z';
		case KeyCode::Key::Num0:       return '0';
		case KeyCode::Key::Num1:       return '1';
		case KeyCode::Key::Num2:       return '2';
		case KeyCode::Key::Num3:       return '3';
		case KeyCode::Key::Num4:       return '4';
		case KeyCode::Key::Num5:       return '5';
		case KeyCode::Key::Num6:       return '6';
		case KeyCode::Key::Num7:       return '7';
		case KeyCode::Key::Num8:       return '8';
		case KeyCode::Key::Num9:       return '9';
		case KeyCode::Key::Escape:     return VK_ESCAPE;
		case KeyCode::Key::LControl:   return VK_LCONTROL;
		case KeyCode::Key::LShift:     return VK_LSHIFT;
		case KeyCode::Key::LAlt:       return VK_LMENU;
		case KeyCode::Key::LSystem:    return VK_LWIN;
		case KeyCode::Key::RControl:   return VK_RCONTROL;
		case KeyCode::Key::RShift:     return VK_RSHIFT;
		case KeyCode::Key::RAlt:       return VK_RMENU;
		case KeyCode::Key::RSystem:    return VK_RWIN;
		case KeyCode::Key::Menu:       return VK_APPS;
		case KeyCode::Key::LBracket:   return VK_OEM_4;
		case KeyCode::Key::RBracket:   return VK_OEM_6;
		case KeyCode::Key::Semicolon:  return VK_OEM_1;
		case KeyCode::Key::Comma:      return VK_OEM_COMMA;
		case KeyCode::Key::Period:     return VK_OEM_PERIOD;
		case KeyCode::Key::Apostrophe: return VK_OEM_7;
		case KeyCode::Key::Slash:      return VK_OEM_2;
		case KeyCode::Key::Backslash:  return VK_OEM_5;
		case KeyCode::Key::Grave:      return VK_OEM_3;
		case KeyCode::Key::Equal:      return VK_OEM_PLUS;
		case KeyCode::Key::Hyphen:     return VK_OEM_MINUS;
		case KeyCode::Key::Space:      return VK_SPACE;
		case KeyCode::Key::Enter:      return VK_RETURN;
		case KeyCode::Key::Backspace:  return VK_BACK;
		case KeyCode::Key::Tab:        return VK_TAB;
		case KeyCode::Key::PageUp:     return VK_PRIOR;
		case KeyCode::Key::PageDown:   return VK_NEXT;
		case KeyCode::Key::End:        return VK_END;
		case KeyCode::Key::Home:       return VK_HOME;
		case KeyCode::Key::Insert:     return VK_INSERT;
		case KeyCode::Key::Delete:     return VK_DELETE;
		case KeyCode::Key::Add:        return VK_ADD;
		case KeyCode::Key::Subtract:   return VK_SUBTRACT;
		case KeyCode::Key::Multiply:   return VK_MULTIPLY;
		case KeyCode::Key::Divide:     return VK_DIVIDE;
		case KeyCode::Key::Left:       return VK_LEFT;
		case KeyCode::Key::Right:      return VK_RIGHT;
		case KeyCode::Key::Up:         return VK_UP;
		case KeyCode::Key::Down:       return VK_DOWN;
		case KeyCode::Key::Numpad0:    return VK_NUMPAD0;
		case KeyCode::Key::Numpad1:    return VK_NUMPAD1;
		case KeyCode::Key::Numpad2:    return VK_NUMPAD2;
		case KeyCode::Key::Numpad3:    return VK_NUMPAD3;
		case KeyCode::Key::Numpad4:    return VK_NUMPAD4;
		case KeyCode::Key::Numpad5:    return VK_NUMPAD5;
		case KeyCode::Key::Numpad6:    return VK_NUMPAD6;
		case KeyCode::Key::Numpad7:    return VK_NUMPAD7;
		case KeyCode::Key::Numpad8:    return VK_NUMPAD8;
		case KeyCode::Key::Numpad9:    return VK_NUMPAD9;
		case KeyCode::Key::F1:         return VK_F1;
		case KeyCode::Key::F2:         return VK_F2;
		case KeyCode::Key::F3:         return VK_F3;
		case KeyCode::Key::F4:         return VK_F4;
		case KeyCode::Key::F5:         return VK_F5;
		case KeyCode::Key::F6:         return VK_F6;
		case KeyCode::Key::F7:         return VK_F7;
		case KeyCode::Key::F8:         return VK_F8;
		case KeyCode::Key::F9:         return VK_F9;
		case KeyCode::Key::F10:        return VK_F10;
		case KeyCode::Key::F11:        return VK_F11;
		case KeyCode::Key::F12:        return VK_F12;
		case KeyCode::Key::F13:        return VK_F13;
		case KeyCode::Key::F14:        return VK_F14;
		case KeyCode::Key::F15:        return VK_F15;
		case KeyCode::Key::Pause:      return VK_PAUSE;
		default:                       return -1;
		}
	}

	KeyCode::Key Keyboard::ToKeyCode(i32 key)
	{
		switch (key)
		{
		case 'A':           return KeyCode::Key::A;
		case 'B':           return KeyCode::Key::B;
		case 'C':           return KeyCode::Key::C;
		case 'D':           return KeyCode::Key::D;
		case 'E':           return KeyCode::Key::E;
		case 'F':           return KeyCode::Key::F;
		case 'G':           return KeyCode::Key::G;
		case 'H':           return KeyCode::Key::H;
		case 'I':           return KeyCode::Key::I;
		case 'J':           return KeyCode::Key::J;
		case 'K':           return KeyCode::Key::K;
		case 'L':           return KeyCode::Key::L;
		case 'M':           return KeyCode::Key::M;
		case 'N':           return KeyCode::Key::N;
		case 'O':           return KeyCode::Key::O;
		case 'P':           return KeyCode::Key::P;
		case 'Q':           return KeyCode::Key::Q;
		case 'R':           return KeyCode::Key::R;
		case 'S':           return KeyCode::Key::S;
		case 'T':           return KeyCode::Key::T;
		case 'U':           return KeyCode::Key::U;
		case 'V':           return KeyCode::Key::V;
		case 'W':           return KeyCode::Key::W;
		case 'X':           return KeyCode::Key::X;
		case 'Y':           return KeyCode::Key::Y;
		case 'Z':           return KeyCode::Key::Z;
		case '0':           return KeyCode::Key::Num0;
		case '1':           return KeyCode::Key::Num1;
		case '2':           return KeyCode::Key::Num2;
		case '3':           return KeyCode::Key::Num3;
		case '4':           return KeyCode::Key::Num4;
		case '5':           return KeyCode::Key::Num5;
		case '6':           return KeyCode::Key::Num6;
		case '7':           return KeyCode::Key::Num7;
		case '8':           return KeyCode::Key::Num8;
		case '9':           return KeyCode::Key::Num9;
		case VK_ESCAPE:     return KeyCode::Key::Escape;
		case VK_LCONTROL:   return KeyCode::Key::LControl;
		case VK_LSHIFT:     return KeyCode::Key::LShift;
		case VK_LMENU:      return KeyCode::Key::LAlt;
		case VK_LWIN:       return KeyCode::Key::LSystem;
		case VK_RCONTROL:   return KeyCode::Key::RControl;
		case VK_RSHIFT:     return KeyCode::Key::RShift;
		case VK_RMENU:      return KeyCode::Key::RAlt;
		case VK_RWIN:       return KeyCode::Key::RSystem;
		case VK_APPS:       return KeyCode::Key::Menu;
		case VK_OEM_4:      return KeyCode::Key::LBracket;
		case VK_OEM_6:      return KeyCode::Key::RBracket;
		case VK_OEM_1:      return KeyCode::Key::Semicolon;
		case VK_OEM_COMMA:  return KeyCode::Key::Comma;
		case VK_OEM_PERIOD: return KeyCode::Key::Period;
		case VK_OEM_7:      return KeyCode::Key::Apostrophe;
		case VK_OEM_2:      return KeyCode::Key::Slash;
		case VK_OEM_5:      return KeyCode::Key::Backslash;
		case VK_OEM_3:      return KeyCode::Key::Grave;
		case VK_OEM_PLUS:   return KeyCode::Key::Equal;
		case VK_OEM_MINUS:  return KeyCode::Key::Hyphen;
		case VK_SPACE:      return KeyCode::Key::Space;
		case VK_RETURN:     return KeyCode::Key::Enter;
		case VK_BACK:       return KeyCode::Key::Backspace;
		case VK_TAB:        return KeyCode::Key::Tab;
		case VK_PRIOR:      return KeyCode::Key::PageUp;
		case VK_NEXT:       return KeyCode::Key::PageDown;
		case VK_END:        return KeyCode::Key::End;
		case VK_HOME:       return KeyCode::Key::Home;
		case VK_INSERT:     return KeyCode::Key::Insert;
		case VK_DELETE:     return KeyCode::Key::Delete;
		case VK_ADD:        return KeyCode::Key::Add;
		case VK_SUBTRACT:   return KeyCode::Key::Subtract;
		case VK_MULTIPLY:   return KeyCode::Key::Multiply;
		case VK_DIVIDE:     return KeyCode::Key::Divide;
		case VK_LEFT:       return KeyCode::Key::Left;
		case VK_RIGHT:      return KeyCode::Key::Right;
		case VK_UP:         return KeyCode::Key::Up;
		case VK_DOWN:       return KeyCode::Key::Down;
		case VK_NUMPAD0:    return KeyCode::Key::Numpad0;
		case VK_NUMPAD1:    return KeyCode::Key::Numpad1;
		case VK_NUMPAD2:    return KeyCode::Key::Numpad2;
		case VK_NUMPAD3:    return KeyCode::Key::Numpad3;
		case VK_NUMPAD4:    return KeyCode::Key::Numpad4;
		case VK_NUMPAD5:    return KeyCode::Key::Numpad5;
		case VK_NUMPAD6:    return KeyCode::Key::Numpad6;
		case VK_NUMPAD7:    return KeyCode::Key::Numpad7;
		case VK_NUMPAD8:    return KeyCode::Key::Numpad8;
		case VK_NUMPAD9:    return KeyCode::Key::Numpad9;
		case VK_F1:         return KeyCode::Key::F1;
		case VK_F2:         return KeyCode::Key::F2;
		case VK_F3:         return KeyCode::Key::F3;
		case VK_F4:         return KeyCode::Key::F4;
		case VK_F5:         return KeyCode::Key::F5;
		case VK_F6:         return KeyCode::Key::F6;
		case VK_F7:         return KeyCode::Key::F7;
		case VK_F8:         return KeyCode::Key::F8;
		case VK_F9:         return KeyCode::Key::F9;
		case VK_F10:        return KeyCode::Key::F10;
		case VK_F11:        return KeyCode::Key::F11;
		case VK_F12:        return KeyCode::Key::F12;
		case VK_F13:        return KeyCode::Key::F13;
		case VK_F14:        return KeyCode::Key::F14;
		case VK_F15:        return KeyCode::Key::F15;
		case VK_PAUSE:      return KeyCode::Key::Pause;

		default: return KeyCode::Key::Unknown;
		}
	}
}