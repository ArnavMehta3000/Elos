#pragma once
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/String.h>
#include <Elos/Window/WindowHandle.h>
#include <Windows.h>

namespace Elos
{
	enum class MessagBoxButton : u32
	{
		AbortRetryIgnore       = MB_ABORTRETRYIGNORE,
		CancelTryAgainContinue = MB_CANCELTRYCONTINUE,
		Help                   = MB_HELP,
		Ok                     = MB_OK,
		OkCancel               = MB_OKCANCEL,
		RetryCancel            = MB_RETRYCANCEL,
		YesNo                  = MB_YESNO,
		YesNoCancel            = MB_YESNOCANCEL
	};

	enum class MessageBoxIcon : u32
	{
		Error    = MB_ICONERROR,
		Question = MB_ICONQUESTION,
		Warning  = MB_ICONWARNING,
		Info     = MB_ICONINFORMATION
	};

	enum class MessageBoxReturnValue
	{
		Ok       = 1,
		Cancel   = 2,
		Abort    = 3,
		Retry    = 4,
		Ignore   = 5,
		Yes      = 6,
		No       = 7,
		TryAgain = 10,
		Continue = 11
	};

	struct MessageBoxFlags
	{
		MessagBoxButton Button{ MessagBoxButton::Ok };
		MessageBoxIcon Icon{ MessageBoxIcon::Info };

		constexpr u32 GetFlags() const
		{
			return static_cast<u32>(Button) | static_cast<u32>(Icon);
		}

		operator u32() const { return GetFlags(); }
	};

	struct MessageBoxDesc
	{
		WindowHandle Window{ nullptr };
		String Title;
		String Text;
		MessageBoxFlags Flags{};
	};

	inline MessageBoxReturnValue ShowMessageBox(const MessageBoxDesc& desc)
	{
		return static_cast<MessageBoxReturnValue>(
			MessageBoxA(desc.Window, desc.Text.c_str(), desc.Title.c_str(), desc.Flags));
	}
}