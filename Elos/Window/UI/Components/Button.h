#pragma once
#include <Elos/Window/UI/Components/UIElement.h>
#include <functional>

namespace Elos::UI
{
	class Button : public UIElement
	{
	public:
		class Builder
		{
		public:
			Builder() = default;

			Builder& OnClick(std::function<void(Button&)> callback);
			Builder& IsEnabled(bool enabled);
			Builder& SetBorder(bool hasBorder);
			Builder& SetColor(COLORREF color);
			Builder& SetColorHovered(COLORREF color);
			Builder& SetColorPressed(COLORREF color);
			Builder& SetTextColor(COLORREF color);
			Builder& SetPosition(const WindowPosition& position);
			Builder& SetSize(const WindowSize& size);
			Builder& SetText(const String& text);
			std::shared_ptr<Button> Build(std::shared_ptr<Window> parent);

		private:
			std::function<void(Button&)> m_onClick;
			bool                         m_isEnabled    = true;
			bool                         m_hasBorder    = false;
			COLORREF                     m_color        = RGB(200, 200, 200);
			COLORREF                     m_colorHovered = RGB(220, 220, 220);
			COLORREF                     m_colorPressed = RGB(180, 180, 180);
			COLORREF                     m_textColor    = RGB(0, 0, 0);
			WindowPosition               m_position     = { 0, 0 };
			WindowSize                   m_size         = { 100, 30 };
			String                       m_text         = "Button";

			friend class Button;
		};

	public:
		~Button() = default;

		void SetText(const String& text);
		const String& GetText() const;
		void SetEnabled(bool enabled);
		bool IsEnabled() const;
		void SetPosition(const WindowPosition& position);
		WindowPosition GetPosition() const;
		void SetSize(const WindowSize& size);
		WindowSize GetSize();
		void SetOnClick(std::function<void(Button&)> callback);
		void SetColor(COLORREF color);
		void SetColorHovered(COLORREF color);
		void SetColorPressed(COLORREF color);
		void SetTextColor(COLORREF color);
		void Update();
		void Show();
		void Hide();

		inline NODISCARD std::shared_ptr<Window> GetButtonWindow() const { return m_buttonWindow; }

	private:
		Button(std::shared_ptr<Window> parent, const Builder& builder);
		void UpdateAppearance();

	private:
		std::shared_ptr<Window> m_buttonWindow;
		String m_text;
		bool m_isEnabled;
		bool m_hasBorder;

		COLORREF m_color;
		COLORREF m_colorHovered;
		COLORREF m_colorPressed;
		COLORREF m_textColor;

		enum class ButtonState
		{
			Normal,
			Hovered,
			Pressed
		};
		ButtonState m_state = ButtonState::Normal;

		std::function<void(Button&)> m_onClick;
	};
}