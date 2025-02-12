#pragma once

#include <Elos/Common/String.h>
#include <Elos/Window/UI/Widget.h>
#include <Elos/Window/WindowHandle.h>
#include <functional>
#include <Windows.h>

namespace Elos
{
	enum class ButtonStyle
	{
		Default,
		Flat,
		Toggle
	};

	class ELOS_API Button : public Widget
	{
	public:
		Button() = default;
		Button(const String& text, const ButtonStyle& style = ButtonStyle::Default);
		~Button() override;

		void Create(Window& window) override;

		void SetText(const String& text);
		NODISCARD String GetText() const noexcept;

		void SetIsChecked(bool checked);
		bool IsChecked() const noexcept;

		using OnClickCallback = std::function<void()>;
		void SetOnClickCallback(OnClickCallback callback);

		void SetPosition(const WindowPosition& position) override;
		void SetSize(const WindowSize& size) override;
		void SetVisible(bool visible) override;
		void SetEnabled(bool enabled) override;

	private:
		static LRESULT CALLBACK ButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
		void ProcessCommand(WORD notificationCode);

	private:
		WindowHandle    m_handle{ nullptr };
		ButtonStyle     m_style{ ButtonStyle::Default };
		String          m_text;
		OnClickCallback m_onClick;
		bool            m_isChecked{ false };
	};
}