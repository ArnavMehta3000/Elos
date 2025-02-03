#include <Elos/Window/Window.h>
#include <print>

#define USE_HANDLE_EVENT 0

int main()
{
	Elos::Window window("Test Window", { 1280, 720 });

	const auto OnTextInput = [&window](const Elos::Event::TextInput& e)
	{
		std::println("Text Input: {}", e.AsChar());
	};

	const auto OnWindowClose = [&window](const Elos::Event::Closed&)
	{
		window.Close();
	};

	const auto OnEscapePressed = [&window](const Elos::Event::KeyPressed& e)
	{
		if (e.Key == Elos::KeyCode::Escape)
		{
			std::println("Escape Pressed. Closing...");
			window.Close();
		}
	};

	while (window.IsOpen())
	{
#if USE_HANDLE_EVENT
		window.HandleEvents(OnTextInput, OnWindowClose, OnEscapePressed);
#else
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
							std::println("Escape Pressed. Closing...");
							window.Close();
						}
					}
					else if constexpr (std::is_same_v<T, Elos::Event::TextInput>)
					{
						std::println("Text Input: {}", e.AsChar());
					}
				});
		}
#endif
	}
	return 0;
}