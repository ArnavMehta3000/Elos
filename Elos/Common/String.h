#pragma once
#include <Elos/Export.h>
#include <string>
#include <string_view>

namespace Elos
{
	using String      = std::string;
	using StringView  = std::string_view;
	using WString     = std::wstring;
	using WStringView = std::wstring_view;

	ELOS_API WString StringToWString(const String& string);
	ELOS_API String WStringToString(const WString& wstring);
	ELOS_API String HRToString(long hr);
}