#include <Elos/Window/UI/Widget.h>
#include <Elos/Window/Window.h>
#include <algorithm>

namespace Elos
{
	void Widget::Destroy()
	{
		for (auto& child : m_children)
		{
			child->Destroy();
		}

		m_children.clear();
		m_window = nullptr;
	}
	
	void Widget::Update()
	{
		for (auto& child : m_children)
		{
			child->Update();
		}
	}
	
	void Widget::Layout()
	{
		for (auto& child : m_children)
		{
			child->Layout();
		}
	}
	
	void Widget::AddChild(std::unique_ptr<Widget> child)
	{
		if (child)
		{
			child->SetParent(this);
			if (m_window)
			{
				child->Create(*m_window);
			}
			m_children.push_back(std::move(child));
			Layout();
		}
	}
	
	void Widget::RemoveChild(Widget* child)
	{
		auto it = std::find_if(m_children.begin(), m_children.end(),
			[child](const auto& ptr) { return ptr.get() == child; });

		if (it != m_children.end())
		{
			(*it)->Destroy();
			m_children.erase(it);
			Layout();
		}
	}
	
	void Widget::SetSize(const WindowSize& size)
	{
		m_size = size;
		Layout();
	}
	
	void Widget::SetVisible(bool visible)
	{
		m_isVisible = visible;
		for (auto& child : m_children)
		{
			child->SetVisible(visible);
		}
	}
	
	void Widget::SetEnabled(bool enabled)
	{
		m_isEnabled = enabled;
		for (auto& child : m_children)
		{
			child->SetEnabled(enabled);
		}
	}
}