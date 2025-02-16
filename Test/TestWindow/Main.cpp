#include <Elos/Window/Window.h>
#include <Elos/Window/Utils/MessageBox.h>
#include <Elos/Window/Utils/WindowExtensions.h>
#include <print>


int main()
{
	try
	{
		auto mainWindow = std::make_shared<Elos::Window>(
			Elos::WindowCreateInfo::Default("Main Window", { 1280, 720 }));

		auto settingsWindow = mainWindow->CreateChild(
			Elos::WindowCreateInfo::ChildModal(mainWindow, "Test Modal", { 400, 300 }));

		auto gameView = mainWindow->CreateChild(
			Elos::WindowCreateInfo::ChildEmbedded(mainWindow, "Game View", { 800, 600 }));
		gameView->SetMinimumSize({ 100, 100 });
		gameView->SetBackgroundColor(RGB(16, 18, 24));

		bool isDarkTheme = false;
		bool hasRoundCorners = true;
		bool isTransparent = true;

#pragma region Event Handlers
		const auto OnTextInput = [&mainWindow](const Elos::Event::TextInput& e)
			{
				std::println("Text Input: {}", e.AsChar());
			};

		const auto OnWindowClose = [&mainWindow](const Elos::Event::Closed&)
			{
				const Elos::MessageBoxDesc desc
				{
					.Window = mainWindow->GetHandle(),
					.Title = "Close Window Requested",
					.Text = "Are you sure you want to close the window?",
					.Flags = Elos::MessageBoxFlags{}
				};

				const Elos::MessageBoxReturnValue value = Elos::ShowMessageBox(desc);
				if (value == Elos::MessageBoxReturnValue::Ok)
				{
					mainWindow->Close();
				}
			};

		const auto OnKeyPressed = [&mainWindow, &isDarkTheme, &hasRoundCorners, &isTransparent](const Elos::Event::KeyPressed& e)
			{
				if (e.Key == Elos::KeyCode::Escape)
				{
					std::println("Escape Pressed. Closing...");
					mainWindow->Close();
				}

				if (e.Key == Elos::KeyCode::D)
				{
					isDarkTheme = !isDarkTheme;
					std::println("Set window dark theme: {}", isDarkTheme);
					Elos::WindowExtensions::EnableDarkMode(mainWindow->GetHandle(), isDarkTheme);
				}

				if (e.Key == Elos::KeyCode::C)
				{
					hasRoundCorners = !hasRoundCorners;
					std::println("Set window round corners: {}", hasRoundCorners);
					Elos::WindowExtensions::RoundCorners(mainWindow->GetHandle(), hasRoundCorners ?
						Elos::WindowExtensions::CornerPreference::Round : Elos::WindowExtensions::CornerPreference::DoNotRound);
				}

				if (e.Key == Elos::KeyCode::P)
				{
					isTransparent = !isTransparent;
					std::println("Set window transparent: {}", isTransparent);
					Elos::WindowExtensions::SetTransparency(mainWindow->GetHandle(), static_cast<Elos::byte>(isTransparent ? 128 : 255));
				}
			};
#pragma endregion


		while (mainWindow->IsOpen())
		{
			mainWindow->HandleEvents(OnTextInput, OnWindowClose, OnKeyPressed,
				[](const Elos::Event::MouseButtonPressed& e)
				{
					std::println("Main Window clicked at position: ({}, {})", e.X, e.Y);
				});
			
			// Handle game view events
			if (gameView && gameView->IsOpen())
			{
				if (gameView->IsOpen())
				{
					gameView->HandleEvents(
						[](const Elos::Event::MouseButtonPressed& e)
						{
							std::println("Game view clicked at position: ({}, {})", e.X, e.Y);
						},
						[](const Elos::Event::Resized& e)
						{
							std::println("Game view resized to: ({}, {})", e.Size.Width, e.Size.Height);
						}
					);
				}
			}

			// Handle settings window events
			if (settingsWindow && settingsWindow->IsOpen())
			{
				settingsWindow->HandleEvents(
					[&](const Elos::Event::Closed&)
					{
						settingsWindow->Close();
					},
					[](const Elos::Event::KeyPressed&)
					{
						// Handle settings window key presses
						std::println("Settings window key pressed");
					}
				);
			}


			if (mainWindow) mainWindow->Redraw();
			if (gameView) gameView->Redraw();
			if (settingsWindow) settingsWindow->Redraw();
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
		std::println("{}", e.what());
	}
	return 0;
}