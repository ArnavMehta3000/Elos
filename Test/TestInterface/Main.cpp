#include <Elos/Common/FunctionMacros.h>
#include <Elos/Interface/TaggedPtr.h>
#include <print>
#include <string>
#include <cassert>


// ----- SIMPLE INTERFACING -----


ELOS_DECLARE_INTERFACE(IRenderable,
	{ t.Render() } -> std::same_as<void>;
	{ t.IsVisible() } -> std::same_as<bool>;
);

ELOS_DECLARE_INTERFACE(ISerializable,
	{ t.Serialize() } -> std::convertible_to<std::string>;
	{ t.Deserialize(std::declval<const std::string&>()) } -> std::same_as<bool>;
);

class GameObject : public Elos::Interface<IRenderable, ISerializable>
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


// ----- USING TAGGED PTR WITH INTERFACE-----

ELOS_DECLARE_INTERFACE(ICounter,
	{ t.Increment() } -> std::same_as<void>;
	{ t.GetCount() } -> std::same_as<int>;
);

ELOS_DECLARE_INTERFACE(IResettable,
	{ t.Reset() } -> std::same_as<void>;
);


class SimpleCounter : public Elos::Interface<ICounter>
{
public:
	void Increment() { m_count++; }
	int GetCount() { return m_count; }
private:
	int m_count = 0;
};

class AdvancedCounter : public Elos::Interface<ICounter, IResettable>
{
public:
	void Increment() { m_count++; }
	int GetCount() { return m_count; }
	void Reset() { m_count = 0; }

private:
	int m_count = 0;
};

ELOS_VERIFY_INTERFACE(ICounter, SimpleCounter);
ELOS_VERIFY_INTERFACE(ICounter, AdvancedCounter);
ELOS_VERIFY_INTERFACE(IResettable, AdvancedCounter);

// Helper to track destructor calls
struct DestructorTracker
{
	static int s_destroyCount;
	~DestructorTracker()
	{
		s_destroyCount++;
	}
};
int DestructorTracker::s_destroyCount = 0;

class TrackedCounter : public SimpleCounter, public DestructorTracker {};


int main()
{
	GameObject o;

	DoRender(o);
	DoSerialize(o);

	// ----- Test TaggedPtr with Interface -----
	using TestPtr = Elos::TaggedPtr<SimpleCounter, AdvancedCounter>;

	const auto TestBasicTypeChecks = []()
	{
		std::println("Testing basic type checks");


		TestPtr empty;
		assert(!bool(empty) && "Empty ptr should be falsey");

		// Create a simple counter
		auto simple = Elos::MakeTagged<SimpleCounter, TestPtr::Pack>();
		assert(bool(simple) && "Simple pointer should be valid");
		assert(simple.Is<SimpleCounter>() && "Should be SimpleCounter");
		assert(!simple.Is<AdvancedCounter>() && "Should not be AdvancedCounter");
		assert(simple.Tag() == TestPtr::TypeIndex<SimpleCounter>() && "Tag should match SimpleCounter index");


		auto advanced = Elos::MakeTagged<AdvancedCounter, TestPtr::Pack>();
		assert(bool(advanced) && "Advanced pointer should be valid");
		assert(advanced.Is<AdvancedCounter>() && "Should be AdvancedCounter");
		assert(!advanced.Is<SimpleCounter>() && "Should not be SimpleCounter");

		std::println("Basic type checks passed!");
	};

	const auto TestCasting = []()
	{
		std::println("Testing casting");

		auto ptr = Elos::MakeTagged<SimpleCounter, TestPtr::Pack>();

		auto simplePtr = ptr.Cast<SimpleCounter>();
		assert(simplePtr != nullptr && "Cast to correct type should succeed");

		auto advancedPtr = ptr.Cast<AdvancedCounter>();
		assert(advancedPtr == nullptr && "Cast to wrong type should fail");

		std::println("Casting passed!");
	};

	const auto TestInterfaceDispatch = []()
	{
		std::println("Testing interface dispatch");

		auto simple = Elos::MakeTagged<SimpleCounter, TestPtr::Pack>();
		auto advanced = Elos::MakeTagged<AdvancedCounter, TestPtr::Pack>();

		simple.Dispatch<ICounter>([](auto& ptr)
		{
			ptr->Increment();
			assert(ptr->GetCount() == 1 && "Count should be 1");
		});

		advanced.Dispatch<ICounter>([](auto& ptr)
		{
			ptr->Increment();
			ptr->Increment();
			assert(ptr->GetCount() == 2 && "Count should be 2");
		});

		advanced.Dispatch<IResettable>([](auto& ptr)
		{
			ptr->Reset();
			assert(ptr->GetCount() == 0 && "Count should be 0 after reset");
		});

		// This would throw a runtime error if uncommented:
		//simple.Dispatch<IResettable>([](auto& ptr) { ptr->Reset(); });

		std::println("Interface dispatch passed!");
	};

	const auto TestSharedPointerSemantics = []()
	{
		std::println("Testing shared pointer semantics");
		using TestPtr = Elos::TaggedPtr<SimpleCounter, AdvancedCounter>;
		//auto ptr1 = Elos::MakeTagged<SimpleCounter, SimpleCounter, AdvancedCounter>(new SimpleCounter());

		TestPtr ptr1 = Elos::MakeTagged<SimpleCounter, TestPtr::Pack>();
		auto ptr2 = ptr1;  // shared copy

		ptr1.Dispatch<ICounter>([](auto& ptr)
		{
			ptr->Increment();
		});

		ptr2.Dispatch<ICounter>([](auto& ptr)
		{
			assert(ptr->GetCount() == 1 && "Count should be shared between pointers");
		});

		std::println("Shared pointer semantics tests passed!");
	};

	const auto TestAutomaticCleanup = []()
	{
		std::println("Testing automatic cleanup");

		DestructorTracker::s_destroyCount = 0;
		{
			using TrackedPtr = Elos::TaggedPtr<TrackedCounter, AdvancedCounter>;
			auto ptr = Elos::MakeTagged<TrackedCounter, TrackedPtr::Pack>();
		}

		assert(DestructorTracker::s_destroyCount == 1 && "Should have cleaned up automatically");

		std::println("Automatic cleanup passed!");
	};


	TestBasicTypeChecks();
	TestCasting();
	TestInterfaceDispatch();
	TestSharedPointerSemantics();
	TestAutomaticCleanup();

	return 0;
}
