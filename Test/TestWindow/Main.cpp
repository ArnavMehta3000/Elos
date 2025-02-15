#include <Elos/Window/Window.h>
#include <Elos/Window/Utils/MessageBox.h>
#include <Elos/Window/Utils/WindowExtensions.h>
#include <print>

#define USE_HANDLE_EVENT 1

int main()
{
	Elos::Window mainWindow(Elos::WindowCreateInfo{
		.Title = "Game Editor",
		.Size = {1280, 720}
	});

	Elos::Window* const gameView = mainWindow.CreateChild({
		.Title = "Game View",
		.Size = {640, 480},
		.ChildMode = Elos::WindowChildMode::Embedded
	});

	// Create a modal settings window
	Elos::Window* const settingsWindow = mainWindow.CreateChild({
		.Title = "Settings",
		.Size = {400, 300},
		.ChildMode = Elos::WindowChildMode::Modal
	});



	bool isDarkTheme = false;
	bool hasRoundCorners = true;
	bool isTransparent = true;

	const auto OnTextInput = [&mainWindow](const Elos::Event::TextInput& e)
	{
		std::println("Text Input: {}", e.AsChar());
	};

	const auto OnWindowClose = [&mainWindow](const Elos::Event::Closed&)
	{
		const Elos::MessageBoxDesc desc
		{
			.Window = mainWindow.GetHandle(),
			.Title = "Close Window Requested",
			.Text = "Are you sure you want to close the window?",
			.Flags = Elos::MessageBoxFlags{}
		};
		
		const Elos::MessageBoxReturnValue value = Elos::ShowMessageBox(desc);
		if (value == Elos::MessageBoxReturnValue::Ok)
		{
			mainWindow.Close();
		}
	};

	const auto OnKeyPressed = [&mainWindow, &isDarkTheme, &hasRoundCorners, &isTransparent](const Elos::Event::KeyPressed& e)
	{
		if (e.Key == Elos::KeyCode::Escape)
		{
			std::println("Escape Pressed. Closing...");
			mainWindow.Close();
		}
		
		if (e.Key == Elos::KeyCode::D)
		{
			isDarkTheme = !isDarkTheme;
			std::println("Set window dark theme: {}", isDarkTheme);
			Elos::WindowExtensions::EnableDarkMode(mainWindow.GetHandle(), isDarkTheme);
		}

		if (e.Key == Elos::KeyCode::C)
		{
			hasRoundCorners = !hasRoundCorners;
			std::println("Set window round corners: {}", hasRoundCorners);
			Elos::WindowExtensions::RoundCorners(mainWindow.GetHandle(), hasRoundCorners ?
				Elos::WindowExtensions::CornerPreference::Round : Elos::WindowExtensions::CornerPreference::DoNotRound);
		}

		if (e.Key == Elos::KeyCode::P)
		{
			isTransparent = !isTransparent;
			std::println("Set window transparent: {}", isTransparent);
			Elos::WindowExtensions::SetTransparency(mainWindow.GetHandle(), static_cast<Elos::byte>(isTransparent ? 128 : 255));
		}
	};


	while (mainWindow.IsOpen())
	{
#if USE_HANDLE_EVENT
		mainWindow.HandleEvents(OnTextInput, OnWindowClose, OnKeyPressed);
#else
		while (const auto event = window.PollEvent())
		{
			if (event->Is<Elos::Event::Closed>())
			{
				mainWindow.Close();
			}
		
			event->visit(
				[&mainWindow](const auto& e)
				{
					using T = std::decay_t<decltype(e)>;
					if constexpr (std::is_same_v<T, Elos::Event::KeyReleased>)
					{
						if (e.Key == Elos::KeyCode::Escape)
						{
							std::println("Escape Pressed. Closing...");
							mainWindow.Close();
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