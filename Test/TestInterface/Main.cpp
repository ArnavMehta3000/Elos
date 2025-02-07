#include <Elos/Common/FunctionMacros.h>
#include <Elos/Interface/Interface.h>
#include <string>
#include <print>

ELOS_DECLARE_INTERFACE(IRenderable,
	{ t.Render() } -> std::same_as<void>;
	{ t.IsVisible() } -> std::same_as<bool>;
);

ELOS_DECLARE_INTERFACE(ISerializable,
	{ t.Serialize() } -> std::convertible_to<std::string>;
	{ t.Deserialize(std::declval<const std::string&>()) } -> std::same_as<bool>;
);

class GameObject : public Elos::Interface<GameObject, IRenderable, ISerializable>
{
public:
	void Render() {}
	bool IsVisible() { return true; }
	std::string Serialize() { return "data"; }
	bool Deserialize(MAYBE_UNUSED const std::string& data) { return true; }
};

ELOS_VERIFY_INTERFACE(IRenderable, GameObject);
ELOS_VERIFY_INTERFACE(ISerializable, GameObject);


template <Elos::ImplementsInterface<IRenderable> T>
void DoRender(T& renderable)
{
	renderable.Render();
	MAYBE_UNUSED bool isVisible = renderable.IsVisible();
}

template <IsISerializable T>
void DoSerialize(T& serializable)
{
	std::string data = serializable.Serialize();
	MAYBE_UNUSED bool success = serializable.Deserialize(data);
}


int main()
{
	GameObject o;
	
	DoRender(o);
	DoSerialize(o);

	return 0;
}