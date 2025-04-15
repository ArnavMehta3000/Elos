#include <Elos/Window/Utils/WindowExtensions.h>
#include <Windows.h>
#include <dwmapi.h>

namespace Elos
{
	namespace Internal
	{
		// Ref: https://stackoverflow.com/questions/57626018/how-to-get-a-blurred-translucent-qml-window-similar-to-fluent-design-guidelines

		typedef enum _WINDOWCOMPOSITIONATTRIB
		{
			WCA_UNDEFINED                     = 0,
			WCA_NCRENDERING_ENABLED           = 1,
			WCA_NCRENDERING_POLICY            = 2,
			WCA_TRANSITIONS_FORCEDISABLED     = 3,
			WCA_ALLOW_NCPAINT                 = 4,
			WCA_CAPTION_BUTTON_BOUNDS         = 5,
			WCA_NONCLIENT_RTL_LAYOUT          = 6,
			WCA_FORCE_ICONIC_REPRESENTATION   = 7,
			WCA_EXTENDED_FRAME_BOUNDS         = 8,
			WCA_HAS_ICONIC_BITMAP             = 9,
			WCA_THEME_ATTRIBUTES              = 10,
			WCA_NCRENDERING_EXILED            = 11,
			WCA_NCADORNMENTINFO               = 12,
			WCA_EXCLUDED_FROM_LIVEPREVIEW     = 13,
			WCA_VIDEO_OVERLAY_ACTIVE          = 14,
			WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
			WCA_DISALLOW_PEEK                 = 16,
			WCA_CLOAK                         = 17,
			WCA_CLOAKED                       = 18,
			WCA_ACCENT_POLICY                 = 19,
			WCA_FREEZE_REPRESENTATION         = 20,
			WCA_EVER_UNCLOAKED                = 21,
			WCA_VISUAL_OWNER                  = 22,
			WCA_HOLOGRAPHIC                   = 23,
			WCA_EXCLUDED_FROM_DDA             = 24,
			WCA_PASSIVEUPDATEMODE             = 25,
			WCA_LAST                          = 26
		} WINDOWCOMPOSITIONATTRIB;

		typedef struct _WINDOWCOMPOSITIONATTRIBDATA
		{
			WINDOWCOMPOSITIONATTRIB Attrib;
			PVOID pvData;
			SIZE_T cbData;
		} WINDOWCOMPOSITIONATTRIBDATA;

		typedef enum _ACCENT_STATE
		{
			ACCENT_DISABLED                   = 0,
			ACCENT_ENABLE_GRADIENT            = 1,
			ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
			ACCENT_ENABLE_BLURBEHIND          = 3,
			ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4, // RS4 1803
			ACCENT_ENABLE_HOSTBACKDROP        = 5, // RS5 1809
			ACCENT_INVALID_STATE              = 6
		} ACCENT_STATE;

		typedef struct _ACCENT_POLICY
		{
			ACCENT_STATE AccentState;
			DWORD AccentFlags;
			DWORD GradientColor;
			DWORD AnimationId;
		} ACCENT_POLICY;

		typedef BOOL(WINAPI* pfnGetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

		typedef BOOL(WINAPI* pfnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
	}

	void WindowExtensions::EnableAcrylicBlur(const WindowHandle& windowHandle)
	{
		if (windowHandle)
		{
			using namespace Elos::Internal;

			if (HMODULE user32Dll = ::GetModuleHandle(L"user32.dll"))
			{
				if (pfnGetWindowCompositionAttribute setWindowCompositionAttribute = (pfnGetWindowCompositionAttribute)::GetProcAddress(user32Dll, "SetWindowCompositionAttribute"))
				{
					ACCENT_POLICY policy = { ACCENT_ENABLE_BLURBEHIND, 0, 0, 0 };
					WINDOWCOMPOSITIONATTRIBDATA data = { WCA_ACCENT_POLICY, &policy, sizeof(policy) };

					setWindowCompositionAttribute(windowHandle, &data);
					::SetWindowPos(windowHandle, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
				}
			}
		}
	}
	
	void WindowExtensions::SetTransparency(const WindowHandle& windowHandle, const byte alpha)
	{
		if (windowHandle)
		{
			LONG style = GetWindowLong(windowHandle, GWL_EXSTYLE);
			::SetWindowLong(windowHandle, GWL_EXSTYLE, style | WS_EX_LAYERED);
			::SetLayeredWindowAttributes(windowHandle, 0, static_cast<BYTE>(alpha), LWA_ALPHA);
			::SetWindowPos(windowHandle, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}
	
	void WindowExtensions::RoundCorners(const WindowHandle& windowHandle, const CornerPreference preference)
	{
		if (windowHandle)
		{
			DwmSetWindowAttribute(windowHandle, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
			::SetWindowPos(windowHandle, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}
	
	void WindowExtensions::EnableDarkMode(const WindowHandle& windowHandle, const bool enable)
	{
		if (windowHandle)
		{
			BOOL isDarkMode = enable;

			::DwmSetWindowAttribute(windowHandle, DWMWA_USE_IMMERSIVE_DARK_MODE, &isDarkMode, sizeof(isDarkMode));
			::SetWindowPos(windowHandle, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}
	
	void WindowExtensions::RemoveBorder(const WindowHandle& windowHandle)
	{
		// Get current styles
		LONG_PTR style = ::GetWindowLongPtr(windowHandle, GWL_STYLE);
		LONG_PTR exStyle = ::GetWindowLongPtr(windowHandle, GWL_EXSTYLE);

		// Clear all border and frame styles first
		style &= ~(WS_THICKFRAME | WS_BORDER | WS_DLGFRAME);
		exStyle &= ~(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME);

		// Apply the new styles
		::SetWindowLongPtr(windowHandle, GWL_STYLE, style);
		::SetWindowLongPtr(windowHandle, GWL_EXSTYLE, exStyle);

		// Update the window after changing styles
		::SetWindowPos(windowHandle, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
	
	void WindowExtensions::AddBorder(const WindowHandle& windowHandle, bool isResizable)
	{
		// Get current styles
		LONG_PTR style = ::GetWindowLongPtr(windowHandle, GWL_STYLE);
		LONG_PTR exStyle = ::GetWindowLongPtr(windowHandle, GWL_EXSTYLE);

		// Clear all border and frame styles first
		style &= ~(WS_THICKFRAME | WS_BORDER | WS_DLGFRAME);
		exStyle &= ~(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME);

		if (isResizable)
		{
			style |= WS_THICKFRAME;  // Ressizable with border
		}
		else
		{
			style |= WS_BORDER;  // Non-ressizable with border
			exStyle |= WS_EX_CLIENTEDGE;
		}

		// Apply the new styles
		::SetWindowLongPtr(windowHandle, GWL_STYLE, style);
		::SetWindowLongPtr(windowHandle, GWL_EXSTYLE, exStyle);
	}
}