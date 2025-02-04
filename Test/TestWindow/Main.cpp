#include <Elos/Window/Window.h>
#include <print>

#define USE_HANDLE_EVENT 1

int main()
{
	Elos::Window window("Test Window", { 1280, 720 }, Elos::WindowStyle::Default);
	bool isDarkTheme = false;

	const auto OnTextInput = [&window](const Elos::Event::TextInput& e)
	{
		std::println("Text Input: {}", e.AsChar());
	};

	const auto OnWindowClose = [&window](const Elos::Event::Closed&)
	{
		window.Close();
	};

	const auto OnEscapePressed = [&window, &isDarkTheme](const Elos::Event::KeyPressed& e)
	{
		if (e.Key == Elos::KeyCode::Escape)
		{
			std::println("Escape Pressed. Closing...");
			window.Close();
		}
		if (e.Key == Elos::KeyCode::T)
		{
			isDarkTheme = !isDarkTheme;
			window.SetWindowDarkTheme(isDarkTheme);
		}
	};

	const auto OnMouseMoveRaw = [&window](const Elos::Event::MouseMovedRaw& e)
	{
		std::println("Mouse Moved Raw: {} {}", e.DeltaX, e.DeltaY);
	};

	while (window.IsOpen())
	{
#if USE_HANDLE_EVENT
		window.HandleEvents(OnTextInput, OnWindowClose, OnEscapePressed, OnMouseMoveRaw);
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