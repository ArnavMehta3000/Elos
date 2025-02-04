#pragma once
#include <Elos/Common/String.h>
#include <type_traits>
#include <string_view>
#include <tuple>

namespace Elos
{
	// Base class for all interfaces
	struct InterfaceTag {};

	// Check if a type is an interface
	template<typename T>
	struct IsInterface : std::is_base_of<InterfaceTag, T>{};

	template <typename T>
	concept IsInterface_v = IsInterface<T>::value;

	// Interface requirement checker
	template<typename T, typename Interface>
	concept ImplementsInterface = requires
	{
		requires ::Elos::IsInterface_v<Interface>;
		requires std::is_base_of_v<Interface, T>;
	};
	
	/*
	* Base class for all interfaces
	* Expects a concept with name Is<Interface> to already be defined otherwise
	* derived classes will not compile and the verification will fail
	* Always use `ELOS_VERIFY_INTERFACE(Interface, Type)` to ensure that a type implements an interface
	*/
	template <typename Derived, typename... Interfaces>
	class Interface : public Interfaces...
	{
		static_assert((::Elos::IsInterface_v<Interfaces> && ...), 
			"All interfaces must be declared using ELOS_DECLARE_INTERFACE");
	public:
		using InterfaceTypes = std::tuple<Interfaces...>;

	protected:
		~Interface() = default;
	};

// Declare an interface
#define ELOS_DECLARE_INTERFACE(Name, ConceptRequirements)          \
	template <typename T> concept Is##Name = requires(T t) \
	{                                                      \
		ConceptRequirements                                \
	};                                                     \
	struct Name : public ::Elos::InterfaceTag {}

// Verify that a type completely implements an interface
#define ELOS_VERIFY_INTERFACE(Interface, Type) \
	static_assert(Is##Interface<Type>, \
		#Type " does not completely implement \'" #Interface "\'! See interface [" #Interface "] for more requirements.")
}
