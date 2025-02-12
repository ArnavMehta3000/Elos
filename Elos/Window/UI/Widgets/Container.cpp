#include <Elos/Window/UI/Widgets/Container.h>

namespace Elos
{
	Container::Container(Direction direction, u32 spacing)
		: m_direction(direction)
		, m_spacing(spacing)
	{
	}
	
	void Container::Create(Window& window)
	{
		m_window = &window;
		for (auto& child : m_children)
		{
			child->Create(window);
		}
		Layout();
	}
	
	void Container::Layout()
	{
		if (m_children.empty())
			return;

		WindowPosition currentPos = m_position;

		if (m_direction == Direction::Horizontal)
		{
			// Calculate total width of fixed-size widgets and count flexible ones
			u32 totalFixedWidth = 0;
			u32 totalSpacing = m_spacing * (static_cast<u32>(m_children.size()) - 1);
			size_t flexibleCount = 0;

			for (const auto& child : m_children)
			{
				WindowSize childSize = child->GetSize();
				if (childSize.Width > 0)
				{
					totalFixedWidth += childSize.Width;
				}
				else
				{
					flexibleCount++;
				}
			}

			// Calculate remaining space for flexible widgets
			u32 remainingWidth = m_size.Width > (totalFixedWidth + totalSpacing) ?
				m_size.Width - totalFixedWidth - totalSpacing : 0;
			u32 flexibleWidth = static_cast<u32>(flexibleCount) > 0 ? remainingWidth / static_cast<u32>(flexibleCount) : 0;

			// Position and size each child
			for (auto& child : m_children)
			{
				WindowSize childSize = child->GetSize();
				if (childSize.Width == 0)
				{
					childSize.Width = flexibleWidth;
				}
				childSize.Height = m_size.Height;

				child->SetPosition(currentPos);
				child->SetSize(childSize);

				currentPos.X += childSize.Width + m_spacing;
			}
		}
		else // Vertical
		{
			// Calculate total height of fixed-size widgets and count flexible ones
			u32 totalFixedHeight = 0;
			u32 totalSpacing = m_spacing * (static_cast<u32>(m_children.size()) - 1);
			size_t flexibleCount = 0;

			for (const auto& child : m_children)
			{
				WindowSize childSize = child->GetSize();
				if (childSize.Height > 0)
				{
					totalFixedHeight += childSize.Height;
				}
				else
				{
					flexibleCount++;
				}
			}

			u32 remainingHeight = m_size.Height > (totalFixedHeight + totalSpacing) ?
				m_size.Height - totalFixedHeight - totalSpacing : 0;
			u32 flexibleHeight = static_cast<u32>(flexibleCount) > 0 ? remainingHeight / static_cast<u32>(flexibleCount) : 0;

			for (auto& child : m_children)
			{
				WindowSize childSize = child->GetSize();
				if (childSize.Height == 0)
				{
					childSize.Height = flexibleHeight;
				}
				childSize.Width = m_size.Width;

				child->SetPosition(currentPos);
				child->SetSize(childSize);

				currentPos.Y += childSize.Height + m_spacing;
			}
		}

		// Recursively layout children
		Widget::Layout();
	}
}