#include <Elos/Window/Window.h>
#include <Elos/Window/Utils/WindowExtensions.h>
#include <Elos/Window/UI/Components/Button.h>
#include <print>

int main()
{
	auto window = std::make_shared<Elos::Window>(
		Elos::WindowCreateInfo::Default("Main Window", { 1280, 720 }));

	auto button1 = Elos::UI::Button::Builder()
		.SetText("Normal Button")
		.SetPosition({ 100, 100 })
		.SetSize({ 200, 50 })
		.SetColor(RGB(100, 150, 200))
		.SetColorHovered(RGB(120, 170, 220))
		.SetColorPressed(RGB(80, 130, 180))
		.OnClick([](Elos::UI::Button&)
			{
				std::println("Button 1 clicked!");
			})
		.Build(window);

	auto button2 = Elos::UI::Button::Builder()
		.SetText("Disabled Button")
		.SetPosition({ 100, 200 })
		.SetSize({ 200, 50 })
		.SetBorder(true)
		.SetColor(RGB(200, 100, 100))
		.SetColorHovered(RGB(220, 120, 120))
		.SetColorPressed(RGB(180, 80, 80))
		.IsEnabled(false)
		.OnClick([](Elos::UI::Button&)
			{
				std::println("Button 2 clicked!");
			})
		.Build(window);

	auto toggleButton = Elos::UI::Button::Builder()
		.SetText("Toggle Button 2")
		.SetPosition({ 100, 300 })
		.SetSize({ 200, 50 })
		.SetColor(RGB(100, 200, 100))
		.SetColorHovered(RGB(120, 220, 120))
		.SetColorPressed(RGB(80, 180, 80))
		.OnClick([button2](Elos::UI::Button&)
			{
				button2->SetEnabled(!button2->IsEnabled());
				std::println("Button 3 clicked!");
			})
		.Build(window);

	while (window->IsOpen())
	{
		// Process window events
		window->HandleEvents(
			[&window](const Elos::Event::Closed&) 
			{
				window->Close();
			}
		);

		// Process button events
		button1->Update();
		button2->Update();
		toggleButton->Update();
	}
	
	return 0;
}