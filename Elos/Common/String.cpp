#include <Elos/Common/String.h>
#include <Elos/Common/StandardTypes.h>
#include <Windows.h>
#include <comdef.h>

namespace Elos
{
	WString StringToWString(const String& string)
	{
		i32 cch = ::MultiByteToWideChar(CP_ACP, 0, string.data(), (i32)string.size(), nullptr, 0);

		WString wideStr;
		wideStr.resize(cch);
		::MultiByteToWideChar(CP_ACP, 0, string.data(), (int)string.size(), wideStr.data(), cch);

		return wideStr;
	}

	String WStringToString(const WString& wstring)
	{
		int cch = ::WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), static_cast<int>(wstring.size()), nullptr, 0, nullptr, nullptr);

		String narrowStr;
		narrowStr.resize(cch);
		::WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), static_cast<int>(wstring.size()), &narrowStr[0], cch, nullptr, nullptr);

		return narrowStr;
	}
	
	String HRToString(long hr)
	{
		HRESULT hres = static_cast<HRESULT>(hr);
		_com_error err(hres);
		return WStringToString(err.ErrorMessage());
	}
}