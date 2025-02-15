#include <Elos/Window/Window.h>
#include <Elos/Window/Utils/MessageBox.h>
#include <Elos/Window/Utils/WindowExtensions.h>
#include <print>

#define USE_HANDLE_EVENT 1

int main()
{
	Elos::Window window("Test Window", { 1280, 720 }, Elos::WindowStyle::Default);
	bool isDarkTheme = false;
	bool hasRoundCorners = true;
	bool isTransparent = true;

	const auto OnTextInput = [&window](const Elos::Event::TextInput& e)
	{
		std::println("Text Input: {}", e.AsChar());
	};

	const auto OnWindowClose = [&window](const Elos::Event::Closed&)
	{
		const Elos::MessageBoxDesc desc
		{
			.Window = window.GetHandle(),
			.Title = "Close Window Requested",
			.Text = "Are you sure you want to close the window?",
			.Flags = Elos::MessageBoxFlags{}
		};
		
		const Elos::MessageBoxReturnValue value = Elos::ShowMessageBox(desc);
		if (value == Elos::MessageBoxReturnValue::Ok)
		{
			window.Close();
		}
	};

	const auto OnKeyPressed = [&window, &isDarkTheme, &hasRoundCorners, &isTransparent](const Elos::Event::KeyPressed& e)
	{
		if (e.Key == Elos::KeyCode::Escape)
		{
			std::println("Escape Pressed. Closing...");
			window.Close();
		}
		
		if (e.Key == Elos::KeyCode::D)
		{
			isDarkTheme = !isDarkTheme;
			std::println("Set window dark theme: {}", isDarkTheme);
			Elos::WindowExtensions::EnableDarkMode(window.GetHandle(), isDarkTheme);
		}

		if (e.Key == Elos::KeyCode::C)
		{
			hasRoundCorners = !hasRoundCorners;
			std::println("Set window round corners: {}", hasRoundCorners);
			Elos::WindowExtensions::RoundCorners(window.GetHandle(), hasRoundCorners ? 
				Elos::WindowExtensions::CornerPreference::Round : Elos::WindowExtensions::CornerPreference::DoNotRound);
		}

		if (e.Key == Elos::KeyCode::P)
		{
			isTransparent = !isTransparent;
			std::println("Set window transparent: {}", isTransparent);
			Elos::WindowExtensions::SetTransparency(window.GetHandle(), static_cast<Elos::byte>(isTransparent ? 128 : 255));
		}
	};


	while (window.IsOpen())
	{
#if USE_HANDLE_EVENT
		window.HandleEvents(OnTextInput, OnWindowClose, OnKeyPressed);
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