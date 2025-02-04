#include <Elos/Interface/Interface.h>
#include <string>

template<typename T>
concept IsIRenderable = requires(T& t) 
{
	{ t.Render() } -> std::same_as<void>;
	{ t.IsVisible() } -> std::same_as<bool>;
};
ELOS_DECLARE_INTERFACE(IRenderable);

template<typename T>
concept IsISerializable = requires(T & t) 
{
	{ t.Serialize() } -> std::convertible_to<std::string>;
	{ t.Deserialize(std::declval<const std::string&>()) } -> std::same_as<bool>;
};
ELOS_DECLARE_INTERFACE(ISerializable);


class GameObject : public Elos::Interface<GameObject, IRenderable, ISerializable>
{
public:
	void Render() {}
	bool IsVisible() { return true; }
	std::string Serislize() { return "data"; }
	bool Deserialize(const std::string& data) { return true; }
};

ELOS_VERIFY_INTERFACE(IRenderable, GameObject);
ELOS_VERIFY_INTERFACE(ISerializable, GameObject);


template <Elos::ImplementsInterface<IRenderable> T>
void DoRender(T& renderable)
{
	renderable.Render();
	bool isVisible = renderable.IsVisible();
}

template <Elos::ImplementsInterface<ISerializable> T>
void DoSerialize(T& serializable)
{
	std::string data = serializable.Serialize();
	bool success = serializable.Deserialize(data);
}

int main()
{
	GameObject o;
	
	DoRender(o);
	DoSerialize(o);

	return 0;
}