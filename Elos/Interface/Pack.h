#pragma once
#include <type_traits>

namespace Elos
{
	template <typename... Types>
	struct TypePack
	{
		static constexpr size_t Count = sizeof...(Types);

		template <template <typename...> class Target>
		using Expand = Target<Types...>;

	};

	template <typename T, typename... Ts>
	struct IndexOf 
	{
		static constexpr int Count = 0;
		static_assert(!std::is_same_v<T, T>, "Type not present in TypePack");
	};

	template <typename T, typename... Ts>
	struct IndexOf<T, TypePack<T, Ts...>> 
	{
		static constexpr int Count = 0;
	};

	template <typename T, typename U, typename... Ts>
	struct IndexOf<T, TypePack<U, Ts...>>
	{
		static constexpr int Count = 1 + IndexOf<T, TypePack<Ts...>>::Count;
	};

	template <typename T, typename... Ts>
	struct HasType
	{
		static constexpr bool Value = false;
	};

	template <typename T, typename Tfirst, typename... Ts>
	struct HasType<T, TypePack<Tfirst, Ts...>>
	{
		static constexpr bool Value = (std::is_same_v<T, Tfirst> || HasType<T, TypePack<Ts...>>::Value);
	};
}