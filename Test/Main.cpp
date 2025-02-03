#include <Elos/Window/Window.h>

int main()
{
	Elos::Window window("Test Window", { 1280, 720 });

	while (window.IsOpen())
	{
		while (const auto event = window.PollEvent())
		{
			if (event->Is<Elos::Event::Closed>())
			{
				window.Close();
			}

			event->visit(
				[&window](const auto& e)
				{
					using T = std::decay_t<decltype(e)>;
					if constexpr (std::is_same_v<T, Elos::Event::KeyReleased>)
					{
						if (e.Key == Elos::KeyCode::Escape)
						{
							window.Close();
						}
					}
				});
		}
	}
	return 0;
}