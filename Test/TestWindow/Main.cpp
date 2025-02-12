#include <Elos/Window/Window.h>
#include <Elos/Window/Utils/MessageBox.h>
#include <Elos/Window/UI/Widgets/Button.h>
#include <Elos/Window/UI/Widgets/Container.h>
#include <print>

#define USE_HANDLE_EVENT 1

int main()
{
	Elos::Window window("Test Window", { 1280, 720 }, Elos::WindowStyle::Default);
	bool isDarkTheme = false;

	// Create a vertical container
	auto container = Elos::CreateWidget<Elos::Container>(Elos::Container::Direction::Vertical, 10);

	// Create buttons with explicit sizes
	auto button1 = Elos::CreateWidget<Elos::Button>("Button 1");
	button1->SetSize({ 200, 40 });  // Set explicit size for button
	button1->SetOnClickCallback([]() {
		std::println("Button 1 clicked!");
		});

	auto button2 = Elos::CreateWidget<Elos::Button>("Button 2");
	button2->SetSize({ 200, 40 });  // Set explicit size for button
	button2->SetOnClickCallback([]() {
		std::println("Button 2 clicked!");
		});

	// Add buttons to container
	container->AddChild(std::move(button1));
	container->AddChild(std::move(button2));

	// Set container position to be slightly inset from window edges
	container->SetPosition({ 20, 20 });  // 20 pixels from left and top edges

	// Set container size to match window client area minus margins
	container->SetSize({ 1240, 680 });  // window size minus margins

	window.SetWidget(std::move(container));

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