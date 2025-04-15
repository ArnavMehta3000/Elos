#include <Elos/Window/UI/Components/Button.h>
#include <Elos/Window/Utils/WindowExtensions.h>

namespace Elos::UI
{
	
	Button::Builder& Button::Builder::OnClick(std::function<void(Button&)> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

	Button::Builder& Button::Builder::IsEnabled(bool enabled)
	{
		m_isEnabled = enabled;
		return *this;
	}

	Button::Builder& Button::Builder::SetBorder(bool hasBorder)
	{
		m_hasBorder = hasBorder;
		return *this;
	}

	Button::Builder& Button::Builder::SetColor(COLORREF color)
	{
		m_color = color;
		return *this;
	}

	Button::Builder& Button::Builder::SetColorHovered(COLORREF color)
	{
		m_colorHovered = color;
		return *this;
	}

	Button::Builder& Button::Builder::SetColorPressed(COLORREF color)
	{
		m_colorPressed = color;
		return *this;
	}

	Button::Builder& Button::Builder::SetTextColor(COLORREF color)
	{
		m_textColor = color;
		return *this;
	}

	Button::Builder& Button::Builder::SetPosition(const WindowPosition& position)
	{
		m_position = position;
		return *this;
	}

	Button::Builder& Button::Builder::SetSize(const WindowSize& size)
	{
		m_size = size;
		return *this;
	}

	Button::Builder& Button::Builder::SetText(const String& text)
	{
		m_text = text;
		return *this;
	}
	
	std::shared_ptr<Button> Button::Builder::Build(std::shared_ptr<Window> parent)
	{
		return std::shared_ptr<Button>(new Button(parent, *this));
	}
	
	void Button::SetText(const String& text)
	{
		m_text = text;
		m_buttonWindow->SetTitle(m_text);
	}
		
	const String& Button::GetText() const
	{
		return m_text;
	}
		
	void Button::SetEnabled(bool enabled)
	{
		m_isEnabled = enabled;
		UpdateAppearance();
	}
		
	bool Button::IsEnabled() const
	{
		return m_isEnabled;
	}
		
	void Button::SetPosition(const WindowPosition& position)
	{
		m_buttonWindow->SetPosition(position);
	}
		
	WindowPosition Button::GetPosition() const
	{
		return m_buttonWindow->GetPosition();
	}
		
	void Button::SetSize(const WindowSize& size)
	{
		m_buttonWindow->SetSize(size);
	}

	WindowSize Button::GetSize()
	{
		return m_buttonWindow->GetSize();
	}
		
	void Button::SetOnClick(std::function<void(Button&)> callback)
	{
		m_onClick = std::move(callback);
	}
		
	void Button::SetColor(COLORREF color)
	{
		m_color = color;
		if (m_state == ButtonState::Normal)
		{
			UpdateAppearance();
		}
	}
		
	void Button::SetColorHovered(COLORREF color)
	{
		m_colorHovered = color;
		if (m_state == ButtonState::Hovered)
		{
			UpdateAppearance();
		}
	}
		
	void Button::SetColorPressed(COLORREF color)
	{
		m_colorPressed = color;
		if (m_state == ButtonState::Pressed)
		{
			UpdateAppearance();
		}
	}
		
	void Button::SetTextColor(COLORREF color)
	{
		m_textColor = color;
		// Note: With the current Window implementation, we can't directly set text color
		// This would require extending the Window class to support custom text rendering
	}
		
	void Button::Update()
	{
		m_buttonWindow->HandleEvents(
			[this](const Event::MouseEntered&) 
			{
				if (m_isEnabled && m_state != ButtonState::Pressed)
				{
					m_state = ButtonState::Hovered;
					UpdateAppearance();
				}
			},
			[this](const Event::MouseLeft&) 
			{
				if (m_isEnabled && m_state != ButtonState::Normal)
				{
					m_state = ButtonState::Normal;
					UpdateAppearance();
				}
			},
			[this](const Event::MouseButtonPressed& event) 
			{
				if (m_isEnabled && event.Button == KeyCode::MouseButton::Left)
				{
					m_state = ButtonState::Pressed;
					UpdateAppearance();
				}
			},
			[this](const Event::MouseButtonReleased& event) 
			{
				if (m_isEnabled && event.Button == KeyCode::MouseButton::Left)
				{
					if (m_state == ButtonState::Pressed)
					{
						m_state = ButtonState::Hovered;
						UpdateAppearance();

						// Trigger OnClick
						if (m_onClick)
						{
							m_onClick(*this);
						}
					}
				}
			}
		);
	}

	void Button::Show()
	{
		m_buttonWindow->SetVisible(true);
	}

	void Button::Hide()
	{
		m_buttonWindow->SetVisible(false);
	}
	
	Button::Button(std::shared_ptr<Window> parent, const Builder& builder)
		: UIElement(parent)
		, m_text(builder.m_text)
		, m_isEnabled(builder.m_isEnabled)
		, m_hasBorder(builder.m_hasBorder)
		, m_color(builder.m_color)
		, m_colorHovered(builder.m_colorHovered)
		, m_colorPressed(builder.m_colorPressed)
		, m_textColor(builder.m_textColor)
		, m_onClick(builder.m_onClick)
	{
		WindowCreateInfo createInfo;
		createInfo.Title     = m_text;
		createInfo.Size      = builder.m_size;
		createInfo.Position  = builder.m_position;
		createInfo.Style     = WindowStyle::None;
		createInfo.ChildMode = WindowChildMode::Embedded;
		createInfo.Parent    = parent;

		m_buttonWindow = parent->CreateChild(createInfo);

		// Patch the window style after creation
		if (m_buttonWindow && m_buttonWindow->GetHandle())
		{
			if (m_hasBorder)
			{
				// Add border but do not make the button resizable
				WindowExtensions::AddBorder(m_buttonWindow->GetHandle(), false);
			}
			else
			{
				// Remove border
				WindowExtensions::RemoveBorder(m_buttonWindow->GetHandle());
			}
		}

		UpdateAppearance();
	}

	
	void Button::UpdateAppearance()
	{
		COLORREF color{};

		if (!m_isEnabled)
		{
			// Disabled state - desaturate the normal color
			BYTE r = GetRValue(m_color);
			BYTE g = GetGValue(m_color);
			BYTE b = GetBValue(m_color);

			// Convert to grayscale and reduce intensity
			BYTE gray = static_cast<BYTE>(0.3f * r + 0.59f * g + 0.11f * b);
			color = RGB(gray, gray, gray);
		}
		else
		{
			switch (m_state)
			{
			case ButtonState::Normal:
				color = m_color;
				break;
			case ButtonState::Hovered:
				color = m_colorHovered;
				break;
			case ButtonState::Pressed:
				color = m_colorPressed;
				break;
			}
		}

		m_buttonWindow->SetBackgroundColor(color);
	}
}