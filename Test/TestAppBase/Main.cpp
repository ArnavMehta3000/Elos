#include <Elos/Application/AppBase.h>

class App : public Elos::AppBase
{
public:
	App() : Elos::AppBase() { ConfigureConnections(); }

	void Run()
	{
		while (m_window->IsOpen())
		{
			ProcessWindowEvents();
		}
	}
private:
	void ConfigureConnections()
	{
		SetUpConnection<const Elos::Event::Closed&>(m_windowEventConnections,
			[this](const Elos::Event::Closed&)
			{
				m_window->Close();
			});

		SetUpConnection<const Elos::Event::KeyReleased&>(m_windowEventConnections,
			[this](const Elos::Event::KeyReleased& e)
			{
				if (e.Key == Elos::KeyCode::Escape)
				{
					m_window->Close();
				}
			});
	}

private:
	Elos::WindowEventConnections m_windowEventConnections;
};

int main()
{
	App app;
	app.Run();
	return 0;
}