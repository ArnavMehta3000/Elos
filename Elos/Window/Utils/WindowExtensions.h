#pragma once

#include <Elos/Export.h>
#include <Elos/Common/StandardTypes.h>
#include <Elos/Window/WindowHandle.h>

namespace  Elos
{
	class ELOS_API WindowExtensions
	{
	public:
		enum class CornerPreference
		{
			Default = 0,
			DoNotRound,
			Round,
			RoundSmall
		};

	public:
		static void EnableAcrylicBlur(const WindowHandle& windowHandle);
		static void SetTransparency(const WindowHandle& windowHandle, const byte alpha);
		static void RoundCorners(const WindowHandle& windowHandle, const CornerPreference preference);
		static void EnableDarkMode(const WindowHandle& windowHandle, const bool enable);
		static void RemoveBorder(const WindowHandle& windowHandle);
		static void AddBorder(const WindowHandle& windowHandle, bool isResizable);
	};
}