#pragma once
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/EnumFlags.h>

namespace Elos
{
	enum class WindowStyle : u8
	{
		None     = 0,  // Non-resizable 'splashscreen' style window
		Resize   = 1 << 1,
		Close    = 1 << 2,

		Default = Titlebar | Resize | Close
	};
	ELOS_ENUM_FLAGS(WindowStyle)

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
}