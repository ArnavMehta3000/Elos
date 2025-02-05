#include <Elos/Meta/Reflection.h>
#include <Elos/Interface/Interface.h>
#include <print>

// Simple reflection example
class Component : public Elos::Reflectable<Component>
{
public:
	virtual ~Component() = default;
	virtual void Start() { std::println("Component::Start"); }
	virtual void Update() { std::println("Component::Update"); }

	bool IsEnabled() const { return m_enabled; }
	void SetEnabled(bool value) { m_enabled = value; }

	ELOS_REFLECT_CLASS(Component)
		builder.Function("Start", &Component::Start).IsCallable()
			.Function("Update", &Component::Update).IsCallable()
			.Property("Enabled", &Component::IsEnabled, &Component::SetEnabled);
	ELOS_END_REFLECTION()

private:
	bool m_enabled = true;
};



// Reflection with inheritance example
class Script : public Component, public Elos::Reflectable<Script>
{
public:
	void Start() override { std::println("Script::Start"); }
	void Update() override { std::println("Script::Update"); }

	const Elos::String& GetName() { return m_name; }
	void SetName(const Elos::String& value) { m_name = value; }

	int GetValue() const { return m_value; }

	bool CanExecute() const { return IsEnabled() && !m_name.empty(); }

	ELOS_REFLECT_CLASS(Script)
		builder.Function("Start", &Script::Start).IsCallable(&Script::CanExecute)
			.Function("Update", &Script::Update).IsCallable(&Script::CanExecute)
			.Property("Name", &Script::GetName, &Script::SetName)
			.ReadOnlyProperty("Value", &Script::GetValue);
	ELOS_END_REFLECTION()

private:
	Elos::String m_name;
	int m_value = 0;
};



// Reflection with interface example
ELOS_DECLARE_INTERFACE(IRenderable,
	{ t.Render(std::declval<int>()) } -> std::same_as<void>;
	{ t.IsVisible() } -> std::same_as<bool>;
);

class Sprite : public Elos::Interface<Sprite, IRenderable>, public Elos::Reflectable<Sprite>
{
public:
	void Render(int val)
	{
		std::println("Sprite::Render {}", val);
	}

	bool IsVisible() const
	{
		return m_value % 2 == 0;
	}

	bool CanRender() const { return IsVisible(); }

	void SetValue(int value) { m_value = value; }
	int GetValue() const { return m_value; }

	ELOS_REFLECT_CLASS(Sprite)
		builder.Function("Render", &Sprite::Render).IsCallable(&Sprite::CanRender)
			.Function("IsVisible", &Sprite::IsVisible).IsCallable(&Sprite::CanRender)
			.Property("Value", &Sprite::GetValue, &Sprite::SetValue);
	ELOS_END_REFLECTION()

private:
	int m_value = 0;
};
ELOS_VERIFY_INTERFACE(IRenderable, Sprite);



int main()
{
	{
		Script script;
		script.SetName("MyScript");
		script.SetEnabled(true);

		auto& typeInfo = Elos::Reflectable<Script>::GetTypeInfo();

		// Call function via reflection
		if (auto* startFunc = typeInfo.GetFunction("Start"))
		{
			if (startFunc->Callable(&script))
			{
				startFunc->Invoke(&script, {});
			}
		}

		// Access property via reflection
		if (auto* nameProp = typeInfo.GetProperty("Name"))
		{
			// Get name
			const auto& oldName = nameProp->GetAs<Elos::String>(&script);
			std::println("Old Name: {}", oldName);

			// Set name
			nameProp->Setter(&script, std::string("NewName"));
			std::println("Old Name: {}", script.GetName());
		}
	}


	{
		Sprite sprite;
		auto& typeInfo = Elos::Reflectable<Sprite>::GetTypeInfo();
		auto* renderFunc = typeInfo.GetFunction("Render");

		for (int i = 0; i < 5; i++)
		{
			// Update value
			if (auto* valueProp = typeInfo.GetProperty("Value"))
			{
				valueProp->Setter(&sprite, i);

				auto val = valueProp->GetAs<int>(&sprite);
				std::println("Value set: {}", val);
				
				// Call function
				if (renderFunc->Callable(&sprite))
				{
					renderFunc->Invoke(&sprite, {val});
				}
			}

		}
	}

	return 0;
}