#pragma once
#include <Elos/Window/Window.h>

namespace Elos::UI
{
	class UIElement
	{
	public:
		explicit UIElement(std::shared_ptr<Window> parent) : m_parentWindow(parent) {}

		std::shared_ptr<Window> GetParentWindow() const { return m_parentWindow; }
	private:
		std::shared_ptr<Window> m_parentWindow;
	};
}