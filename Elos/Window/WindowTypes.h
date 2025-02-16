#pragma once
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/EnumFlags.h>
#include <memory>

namespace Elos
{
	enum class WindowStyle : u8
	{
		None     = 0,  // Non-resizable 'splashscreen' style window
		Titlebar = 1 << 0,
		Resize   = 1 << 1,
		Close    = 1 << 2,

		Default = Titlebar | Resize | Close
	};
	ELOS_ENUM_FLAGS(WindowStyle)

	enum class WindowChildMode
	{
		None,      // Not a child window
		Embedded,  // Child window embedded in parent's client area
		Modal,     // Modal window that blocks parent input
		Popup      // Non-modal floating window
	};

	struct WindowSize
	{
		u32 Width;
		u32 Height;
	};

	struct WindowPosition
	{
		i32 X;
		i32 Y;
	};

	struct WindowCreateInfo
	{
		String Title;
		WindowSize Size;
		WindowStyle Style{ WindowStyle::Default };
		WindowChildMode ChildMode{ WindowChildMode::None };
		WindowPosition Position{ static_cast<i32>(0x80000000), static_cast<i32>(0x80000000) };  // Same values as CW_USEDEFAULT
		std::shared_ptr<Window> Parent{ nullptr };

		// Default window create info (non-child, main window)
		static WindowCreateInfo Default(const String& title, const WindowSize& size, WindowStyle style = WindowStyle::Default)
		{
			return WindowCreateInfo
			{
				.Title     = title,
				.Size      = size,
				.Style     = style,
				.ChildMode = WindowChildMode::None,
				.Parent    = nullptr
			};
		}

		// Modal child window
		static WindowCreateInfo ChildModal(const std::shared_ptr<Window>& parent, const String& title, const WindowSize& size, WindowStyle style = WindowStyle::Default)
		{
			return WindowCreateInfo
			{
				.Title     = title,
				.Size      = size,
				.Style     = style,
				.ChildMode = WindowChildMode::Modal,
				.Parent    = parent
			};
		}

		// Window that is embedded in parent window
		static WindowCreateInfo ChildEmbedded(const std::shared_ptr<Window>& parent, const String& title, const WindowSize& size, WindowStyle style = WindowStyle::Default)
		{
			return WindowCreateInfo
			{
				.Title     = title,
				.Size      = size,
				.Style     = style,
				.ChildMode = WindowChildMode::Embedded,
				.Parent    = parent
			};
		}
	};
}