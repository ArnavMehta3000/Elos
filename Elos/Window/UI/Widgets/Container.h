#pragma once

#include <Elos/Window/UI/Widget.h>

namespace Elos
{
	class ELOS_API Container : public Widget
	{
	public:
		enum class Direction { Horizontal, Vertical };

		Container(Direction direction = Direction::Vertical, u32 spacing = 5);

		void Create(Window& window) override;
		void Layout() override;

		void SetSpacing(u32 spacing) { m_spacing = spacing; }
		NODISCARD u32 GetSpacing() const { return m_spacing; }

		void SetDirection(Direction direction) { m_direction = direction; }
		NODISCARD Direction GetDirection() const { return m_direction; }

	private:
		Direction m_direction;
		u32 m_spacing;
	};
}