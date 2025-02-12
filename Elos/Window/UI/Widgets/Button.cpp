#include <Elos/Window/UI/Widgets/Button.h>
#include <Elos/Window/Window.h>
#include <CommCtrl.h>

namespace Elos
{
	Button::Button(const String& text, const ButtonStyle& style)
		: m_text(text)
		, m_style(style)
	{
	}

	Button::~Button()
	{
		if (m_handle)
		{
			::DestroyWindow(m_handle);
			m_handle = nullptr;
		}
	}
	
	void Button::Create(Window& window)
	{
		m_window = &window;

		DWORD win32Style = WS_CHILD | WS_VISIBLE | BS_NOTIFY;

		switch (m_style)
		{
		case ButtonStyle::Flat:
			win32Style |= BS_FLAT;
			break;
		case ButtonStyle::Toggle:
			win32Style |= BS_AUTOCHECKBOX;
			break;
		default:
			win32Style |= BS_PUSHBUTTON;
			break;
		}

		m_handle = ::CreateWindowEx(
			0,
			L"BUTTON",
			StringToWString(m_text).c_str(),
			win32Style,
			m_position.X,
			m_position.Y,
			m_size.Width,
			m_size.Height,
			window.GetHandle(),
			nullptr,
			::GetModuleHandle(NULL),
			nullptr
		);

		if (m_handle)
		{
			::SetWindowSubclass(m_handle, ButtonProc, 0, reinterpret_cast<DWORD_PTR>(this));

			SetVisible(m_isVisible);
			SetEnabled(m_isEnabled);

			if (m_style == ButtonStyle::Toggle)
			{
				SetIsChecked(m_isChecked);
			}
		}
	}
	
	void Button::SetText(const String& text)
	{
		m_text = text;
		if (m_handle)
		{
			::SetWindowText(m_handle, StringToWString(text).c_str());
		}
	}
	
	NODISCARD String Button::GetText() const noexcept
	{
		if (!m_handle)
			return m_text;

		int length = ::GetWindowTextLength(m_handle);
		if (length == 0)
			return {};

		std::wstring buffer(length + 1, L'\0');
		::GetWindowText(m_handle, buffer.data(), length + 1);
		return WStringToString(buffer);
	}
	
	void Button::SetIsChecked(bool checked)
	{
		if (m_style == ButtonStyle::Toggle)
		{
			m_isChecked = checked;
			if (m_handle)
			{
				::SendMessage(m_handle, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
			}
		}
	}
	
	bool Button::IsChecked() const noexcept
	{
		if (m_handle && m_style == ButtonStyle::Toggle)
		{
			return ::SendMessage(m_handle, BM_GETCHECK, 0, 0) == BST_CHECKED;
		}
		return m_isChecked;
	}
	
	void Button::SetOnClickCallback(OnClickCallback callback)
	{
		m_onClick = std::move(callback);
	}
	
	void Button::SetPosition(const WindowPosition& position)
	{
		Widget::SetPosition(position);
		if (m_handle)
		{
			::SetWindowPos(m_handle, nullptr, position.X, position.Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
	}
	
	void Button::SetSize(const WindowSize& size)
	{
		Widget::SetSize(size);
		if (m_handle)
		{
			::SetWindowPos(m_handle, nullptr, 0, 0, size.Width, size.Height, SWP_NOMOVE | SWP_NOZORDER);
		}
	}
	
	void Button::SetVisible(bool visible)
	{
		Widget::SetVisible(visible);
		if (m_handle)
		{
			::ShowWindow(m_handle, visible ? SW_SHOW : SW_HIDE);
		}
	}
	
	void Button::SetEnabled(bool enabled)
	{
		Widget::SetEnabled(enabled);
		if (m_handle)
		{
			::EnableWindow(m_handle, enabled);
		}
	}
	
	LRESULT Button::ButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		Button* button = reinterpret_cast<Button*>(dwRefData);
		if (!button)
			return ::DefSubclassProc(hwnd, msg, wParam, lParam);

		switch (msg)
		{
		case WM_COMMAND:
			button->ProcessCommand(HIWORD(wParam));
			break;

		case WM_NCDESTROY:
			::RemoveWindowSubclass(hwnd, ButtonProc, uIdSubclass);
			break;
		}

		return ::DefSubclassProc(hwnd, msg, wParam, lParam);
	}
	
	void Button::ProcessCommand(WORD notificationCode)
	{
		switch (notificationCode)
		{
		case BN_CLICKED:
			if (m_onClick)
			{
				m_onClick();
			}
			
			if (m_style == ButtonStyle::Toggle)
			{
				m_isChecked = !m_isChecked;
			}
			break;
		}
	}
}