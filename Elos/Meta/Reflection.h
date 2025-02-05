#pragma once
#include <Elos/Common/StandardTypes.h>
#include <Elos/Common/String.h>
#include <unordered_map>
#include <any>
#include <memory>
#include <type_traits>
#include <functional>

namespace Elos
{
	// Forward declarations
	template <typename T> class TypeInfo;
	template <typename T> class ClassBuilder;
	template <typename T> class Reflectable;

	struct Function
	{
		String Name;
		std::function<std::any(void*, const std::vector<std::any>&)> Invoke;
		std::function<bool(void*)> Callable = [](void*) -> bool { return true; };
		std::vector<String> ParamTypes;
		String ReturnType;
	};

	struct Property
	{
		String Name;
		String Type;
		std::function<std::any(void*)> Getter;
		std::function<void(void*, const std::any&)> Setter;
		bool IsReadOnly = false;

		// Helper for getting property value
		template <typename T>
		T GetAs(void* instance) const
		{
			const std::any& value = Getter(instance);
			return std::any_cast<const T&>(value);
		}

		template <typename T>
		const T* GetAsPtr(void* instance) const
		{
			const std::any& value = Getter(instance);
			return std::any_cast<const T>(&value);
		}
	};

	// Auto-registration helper
	template <typename T>
	struct AutoRegister
	{
		AutoRegister()
		{
			T::InitReflection();
		}
	};

	// Type information storage
	template <typename T>
	class TypeInfo
	{
		template<typename> friend class ClassBuilder;

	public:
		TypeInfo(const String& name) : m_typeName(name) {}

		const Function* GetFunction(const String& name) const
		{
			auto it = m_functions.find(name);
			return it != m_functions.end() ? &it->second : nullptr;
		}

		const Property* GetProperty(const String& name) const
		{
			auto it = m_properties.find(name);
			return it != m_properties.end() ? &it->second : nullptr;
		}

		const String& GetName() const { return m_typeName; }

	private:
		String m_typeName;
		std::unordered_map<String, Function> m_functions;
		std::unordered_map<String, Property> m_properties;
	};

	class IReflectable
	{
	public:
		virtual ~IReflectable() = default;
	};

	template <typename Derived>
	class Reflectable : virtual public IReflectable
	{
	public:
		virtual ~Reflectable() = default;

		static TypeInfo<Derived>& GetTypeInfo()
		{
			static TypeInfo<Derived> info(typeid(Derived).name());
			return info;
		}

		static ClassBuilder<Derived>& GetBuilder()
		{
			static ClassBuilder<Derived> builder;
			return builder;
		}

	private:
		static inline AutoRegister<Derived> s_autoRegister{};
	};

	// Function builder for chaining callable conditions
	template<typename Class>
	class FunctionBuilder
	{
	public:
		FunctionBuilder(Function& function, ClassBuilder<Class>& builder)
			: m_function(function), m_classBuilder(builder) {
		}

		ClassBuilder<Class>& IsCallable(bool value = true)
		{
			m_function.Callable = [value](void*) { return value; };
			return m_classBuilder;
		}

		template<typename Func>
		ClassBuilder<Class>& IsCallable(Func Class::* memberFunc)
		{
			m_function.Callable = [memberFunc](void* instance)
				{
					return (static_cast<Class*>(instance)->*memberFunc)();
				};
			return m_classBuilder;
		}

		ClassBuilder<Class>& IsCallable(std::function<bool(void*)> condition)
		{
			m_function.Callable = std::move(condition);
			return m_classBuilder;
		}

	private:
		Function& m_function;
		ClassBuilder<Class>& m_classBuilder;
	};

	// Main builder class for registering class information
	template<typename Class>
	class ClassBuilder
	{
	public:
		ClassBuilder() = default;

		template <typename ReturnType, typename... Args>
		FunctionBuilder<Class> Function(const String& name, ReturnType(Class::* func)(Args...))
		{
			::Elos::Function& f = Reflectable<Class>::GetTypeInfo().m_functions[name];
			f.Name = name;
			f.ReturnType = typeid(ReturnType).name();
			(f.ParamTypes.push_back(typeid(Args).name()), ...);

			f.Invoke = [func](void* instance, const std::vector<std::any>& args) -> std::any
				{
					return InvokeHelper(static_cast<Class*>(instance), func, args,
						std::make_index_sequence<sizeof...(Args)>{});
				};

			return FunctionBuilder<Class>(f, *this);
		}

		template <typename ReturnType, typename... Args>
		FunctionBuilder<Class> Function(const String& name, ReturnType(Class::* func)(Args...) const)
		{
			::Elos::Function& f = Reflectable<Class>::GetTypeInfo().m_functions[name];
			f.Name = name;
			f.ReturnType = typeid(ReturnType).name();
			(f.ParamTypes.push_back(typeid(Args).name()), ...);

			f.Invoke = [func](void* instance, const std::vector<std::any>& args) -> std::any
				{
					return InvokeHelper(static_cast<Class*>(instance), func, args,
						std::make_index_sequence<sizeof...(Args)>{});
				};

			return FunctionBuilder<Class>(f, *this);
		}

		template <typename GetterType, typename SetterType>
		ClassBuilder<Class>& Property(const String& name, GetterType(Class::* getter)(), void(Class::* setter)(SetterType))
		{
			using CleanGetterType = std::remove_reference_t<GetterType>;

			::Elos::Property& p = Reflectable<Class>::GetTypeInfo().m_properties[name];
			p.Name = name;
			p.IsReadOnly = false;
			p.Type = typeid(CleanGetterType).name();

			p.Getter = [getter](void* instance) -> std::any
				{
					return (static_cast<Class*>(instance)->*getter)();
				};

			p.Setter = [setter](void* instance, const std::any& value)
				{
					(static_cast<Class*>(instance)->*setter)(std::any_cast<SetterType>(value));
				};

			return *this;
		}

		template <typename PropType>
		ClassBuilder<Class>& Property(const String& name, PropType(Class::* getter)() const, void(Class::* setter)(PropType))
		{
			::Elos::Property& p = Reflectable<Class>::GetTypeInfo().m_properties[name];
			p.Name = name;
			p.IsReadOnly = false;
			p.Type = typeid(PropType).name();

			p.Getter = [getter](void* instance) -> std::any
				{
					return (static_cast<Class*>(instance)->*getter)();
				};

			p.Setter = [setter](void* instance, const std::any& value)
				{
					(static_cast<Class*>(instance)->*setter)(std::any_cast<PropType>(value));
				};

			return *this;
		}

		template <typename PropType>
		ClassBuilder<Class>& ReadOnlyProperty(const String& name, PropType(Class::* getter)() const)
		{
			::Elos::Property& p = Reflectable<Class>::GetTypeInfo().m_properties[name];
			p.Name = name;
			p.IsReadOnly = true;
			p.Type = typeid(PropType).name();

			p.Getter = [getter](void* instance) -> std::any
				{
					return (static_cast<Class*>(instance)->*getter)();
				};

			return *this;
		}

	private:
		template <typename ReturnType, typename... Args, size_t... I>
		static std::any InvokeHelper(Class* obj, ReturnType(Class::* func)(Args...), const std::vector<std::any>& args, std::index_sequence<I...>)
		{
			return (obj->*func)(std::any_cast<std::remove_cvref_t<Args>>(args[I])...);
		}

		template <typename ReturnType, typename... Args, size_t... I>
		static std::any InvokeHelper(Class* obj, ReturnType(Class::* func)(Args...) const, const std::vector<std::any>& args, std::index_sequence<I...>)
		{
			return (obj->*func)(std::any_cast<std::remove_cvref_t<Args>>(args[I])...);
		}

		template <typename... Args, size_t... I>
		static std::any InvokeHelper(Class* obj, void(Class::* func)(Args...), const std::vector<std::any>& args, std::index_sequence<I...>)
		{
			(obj->*func)(std::any_cast<std::remove_cvref_t<Args>>(args[I])...);
			return std::any();
		}

		template <typename... Args, size_t... I>
		static std::any InvokeHelper(Class* obj, void(Class::* func)(Args...) const, const std::vector<std::any>& args, std::index_sequence<I...>)
		{
			(obj->*func)(std::any_cast<std::remove_cvref_t<Args>>(args[I])...);
			return std::any();
		}
	};
}

#define ELOS_REFLECT_CLASS(Class)\
	friend class ::Elos::ClassBuilder<Class>;\
	static void InitReflection()\
	{\
		static bool initialized = false;\
		if (!initialized)\
		{\
			initialized = true;\
			auto& builder = ::Elos::Reflectable<Class>::GetBuilder();

#define ELOS_END_REFLECTION()\
		}\
	}