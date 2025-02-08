#pragma once

#include <Elos/Math/MathBase.h>
#include <algorithm>

namespace Elos::Internal
{		
	template <DirectXScalar T, std::size_t N>
	struct DirectXVectorTypeMap
	{
		static_assert(N >= 2 && N <= 4, "Vector dimension must be between 2 and 4");
	};

	template<DirectXScalar T, std::size_t N>
	using DirectXVectorType = typename DirectXVectorTypeMap<T, N>::Type;

	template<typename T>
	concept HasDirectXOperations = requires(const T & v, DirectX::XMVECTOR xm) 
	{
		{ v.Load(&v) } -> std::same_as<DirectX::XMVECTOR>;
		{ v.Store(&v, xm) } -> std::same_as<void>;
	};
	
#define DX_TYPE_MAP(T, N, BaseType, Func)                                                                                                                                            \
	template<>                                                                                                                                                                       \
	struct DirectXVectorTypeMap<T, N>                                                                                                                                                \
	{                                                                                                                                                                                \
	public:                                                                                                                                                                          \
		using Type = BaseType;                                                                                                                                                       \
		static constexpr u32 VectorSize = N;                                                                                                                                         \
																																												     \
		static inline auto Load(const Type* v)                                                       { return DirectX::XMLoad##Func(v);                                            } \
		static inline auto Store(Type* dest, DirectX::XMVECTOR v)                                    { DirectX::XMStore##Func(dest, v);                                            } \
		static inline auto Add(DirectX::XMVECTOR lhs, DirectX::XMVECTOR rhs)                         { return DirectX::XMVectorAdd(lhs, rhs);                                      } \
		static inline auto Subtract(DirectX::XMVECTOR lhs, DirectX::XMVECTOR rhs)                    { return DirectX::XMVectorSubtract(lhs, rhs);                                 } \
		static inline auto Multiply(DirectX::XMVECTOR lhs, DirectX::XMVECTOR rhs)                    { return DirectX::XMVectorMultiply(lhs, rhs);                                 } \
		static inline auto Divide (DirectX::XMVECTOR lhs, DirectX::XMVECTOR rhs)                     { return DirectX::XMVectorDivide(lhs, rhs);                                   } \
		static inline auto Scale(DirectX::XMVECTOR v, f32 scalar)                                    { return DirectX::XMVectorScale(v, scalar);                                   } \
		static inline auto Negate(DirectX::XMVECTOR v)                                               { return DirectX::XMVectorNegate(v);                                          } \
		static inline auto MultiplyScalar(DirectX::XMVECTOR v, f32 scalar)                           { return DirectX::XMVectorScale(v, scalar);                                   } \
		static inline auto Dot(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)                           { return DirectX::XMVector##N##Dot(v1, v2);                                   } \
		static inline auto Normalize(DirectX::XMVECTOR v)                                            { return DirectX::XMVector##N##Normalize(v);                                  } \
		static inline auto Length(DirectX::XMVECTOR v)                                               { return DirectX::XMVector##N##Length(v);                                     } \
		static inline auto LengthSquared(DirectX::XMVECTOR v)                                        { return DirectX::XMVector##N##LengthSq(v);                                   } \
		static inline auto Clamp(DirectX::XMVECTOR v, DirectX::XMVECTOR min, DirectX::XMVECTOR max)  { return DirectX::XMVectorClamp(v, min, max);                                 } \
		static inline auto Clamp(DirectX::XMVECTOR v, f32 min, f32 max)                              { return DirectX::XMVector##N##ClampLength(v, min, max);                      } \
		static inline auto Distance(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)                      { return DirectX::XMVector##N##Length(DirectX::XMVectorSubtract(v2, v1));     } \
		static inline auto DistanceSquared(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)               { return DirectX::XMVector##N##LengthSq(DirectX::XMVectorSubtract(v2, v1));   } \
		static inline auto Lerp(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2, f32 t)                   { return DirectX::XMVectorLerp(v1, v2, t);                                    } \
		static inline auto Reflect(DirectX::XMVECTOR incident, DirectX::XMVECTOR normal)             { return DirectX::XMVector##N##Reflect(incident, normal);                     } \
		static inline auto Refract(DirectX::XMVECTOR incident, DirectX::XMVECTOR normal, f32 index)  { return DirectX::XMVector##N##Refract(incident, normal, index);              } \
																																													 \
		template<typename = std::enable_if_t<N <= 3>>                                                                                                                                \
		static inline auto Cross(DirectX::XMVECTOR v1, DirectX::XMVECTOR v2)                                                                                                         \
		{                                                                                                                                                                            \
			if constexpr (N == 2)                                                                                                                                                    \
			{                                                                                                                                                                        \
				return DirectX::XMVector2Cross(v1, v2);                                                                                                                              \
			}                                                                                                                                                                        \
			else if constexpr (N == 3)                                                                                                                                               \
			{                                                                                                                                                                        \
				return DirectX::XMVector3Cross(v1, v2);                                                                                                                              \
			}                                                                                                                                                                        \
		}                                                                                                                                                                            \
																																													 \
		template<typename = std::enable_if_t<N == 4>>                                                                                                                                \
		static inline auto Cross(const DirectX::XMVECTOR& v1, const DirectX::XMVECTOR& v2, const DirectX::XMVECTOR& v3) noexcept                                                     \
		{                                                                                                                                                                            \
			return DirectX::XMVector4Cross(v1, v2, v3);                                                                                                                              \
		}                                                                                                                                                                            \
	}

	DX_TYPE_MAP(f32, 2, DirectX::XMFLOAT2, Float2);
	DX_TYPE_MAP(f32, 3, DirectX::XMFLOAT3, Float3);
	DX_TYPE_MAP(f32, 4, DirectX::XMFLOAT4, Float4);
	DX_TYPE_MAP(i32, 2, DirectX::XMINT2, SInt2);
	DX_TYPE_MAP(i32, 3, DirectX::XMINT3, SInt3);
	DX_TYPE_MAP(i32, 4, DirectX::XMINT4, SInt4);
	DX_TYPE_MAP(u32, 2, DirectX::XMUINT2, UInt2);
	DX_TYPE_MAP(u32, 3, DirectX::XMUINT3, UInt3);
	DX_TYPE_MAP(u32, 4, DirectX::XMUINT4, UInt4);
	
#undef DX_TYPE_MAP

	// Base class for all vectors. Using the right handed coordinate system
	template <DirectXScalar Type, std::size_t N> requires (N >= 2 && N <= 4)
	class VectorBase : public DirectXVectorType<Type, N>
	{
	public:
		using Base = DirectXVectorType<Type, N>;
		using Ops = DirectXVectorTypeMap<Type, N>;

		using Base::Base;
		constexpr VectorBase() noexcept : Base() {}
		VectorBase(DirectX::XMVECTOR v) : Base() { Ops::Store(this, v); }

		// Conversion constructor from other vector types of same dimension
		template<DirectXScalar OtherType>
		explicit constexpr VectorBase(const VectorBase<OtherType, N>& other) noexcept : Base()
		{
			if constexpr (std::is_same_v<Type, OtherType>) 
			{
				*this = other;
			}
			else {
				// Convert each component
				if constexpr (N >= 1) this->x = static_cast<Type>(other.x);
				if constexpr (N >= 2) this->y = static_cast<Type>(other.y);
				if constexpr (N >= 3) this->z = static_cast<Type>(other.z);
				if constexpr (N >= 4) this->w = static_cast<Type>(other.w);
			}
		}

		// Conversion constructor from vector with different dimension
		template<DirectXScalar OtherType, std::size_t M> requires (M >= 2 && M <= 4)
		explicit constexpr VectorBase(const VectorBase<OtherType, M>& other) noexcept : Base()
		{
			constexpr std::size_t MinDim = std::min(N, M);

			// Zero-initialize all components first
			if constexpr (N >= 1) this->x = Type(0);
			if constexpr (N >= 2) this->y = Type(0);
			if constexpr (N >= 3) this->z = Type(0);
			if constexpr (N >= 4) this->w = Type(0);

			// Copy components up to the minimum dimension
			if constexpr (MinDim >= 1) this->x = static_cast<Type>(other.x);
			if constexpr (MinDim >= 2) this->y = static_cast<Type>(other.y);
			if constexpr (MinDim >= 3) this->z = static_cast<Type>(other.z);
			if constexpr (MinDim >= 4) this->w = static_cast<Type>(other.w);

			// For vectors being converted to higher dimensions:
			// - Set w to 1 for points (Vector4)
			// - Set w to 0 for directions (Vector4)
			if constexpr (N == 4 && M < 4)
			{
				this->w = Type(1); // Assuming point by default
			}
		}

		// Implicit conversion operator for same-dimension, same-type vectors
		template<DirectXScalar OtherType> requires std::is_same_v<Type, OtherType>
		constexpr operator VectorBase<OtherType, N>() const noexcept
		{
			return *this;
		}

		// Explicit conversion operator for same-dimension, different-type vectors
		template<DirectXScalar OtherType> requires (!std::is_same_v<Type, OtherType>)
		explicit constexpr operator VectorBase<OtherType, N>() const noexcept
		{
			return VectorBase<OtherType, N>(*this);
		}

		// Explicit conversion operator for different-dimension vectors
		template<DirectXScalar OtherType, std::size_t M> requires (M >= 2 && M <= 4 && M != N)
		explicit constexpr operator VectorBase<OtherType, M>() const noexcept
		{
			return VectorBase<OtherType, M>(*this);
		}


		NODISCARD constexpr operator DirectX::XMVECTOR() const { return Ops::Load(this); }
		NODISCARD constexpr VectorBase& operator=(const DirectX::XMVECTOR& v) { Ops::Store(this, v); return *this; }

		NODISCARD constexpr VectorBase operator+(const VectorBase& rhs) const noexcept { return VectorBase(Ops::Add(*this, rhs));               }		
		NODISCARD constexpr VectorBase operator-(const VectorBase& rhs) const noexcept { return VectorBase(Ops::Subtract(*this, rhs));          }
		NODISCARD constexpr VectorBase operator*(const VectorBase& rhs) const noexcept { return VectorBase(Ops::Multiply(*this, rhs));          }
		NODISCARD constexpr VectorBase operator/(const VectorBase& rhs) const noexcept { return VectorBase(Ops::Divide(*this, rhs));            }
		NODISCARD constexpr VectorBase operator*(f32 scalar) const noexcept            { return VectorBase(Ops::MultiplyScalar(*this, scalar)); }
		NODISCARD constexpr VectorBase operator-() const noexcept                      { return VectorBase(Ops::Negate(*this));                 }
		NODISCARD constexpr VectorBase operator/(Type scalar) const noexcept           { return *this * (Type(1) / scalar);                     }

		NODISCARD constexpr VectorBase& operator+=(const VectorBase& rhs) noexcept { *this = *this + rhs; return *this;     }
		NODISCARD constexpr VectorBase& operator-=(const VectorBase& rhs) noexcept { *this = *this - rhs; return *this;     }
		NODISCARD constexpr VectorBase& operator*=(const VectorBase& rhs) noexcept { *this = *this * rhs; return *this;     }
		NODISCARD constexpr VectorBase& operator/=(const VectorBase& rhs) noexcept { *this = *this / rhs; return *this;     }
		NODISCARD constexpr VectorBase& operator*=(Type scalar) noexcept           { *this = *this * scalar; return *this;  }
		NODISCARD constexpr VectorBase& operator/=(Type scalar) noexcept           { *this = *this / scalar; return *this;  }

		void NormalizeThis() noexcept { *this = Ops::Normalize(*this); }

		NODISCARD constexpr f32 Dot(const VectorBase& rhs) const noexcept                                    { return DirectX::XMVectorGetX(Ops::Dot(*this, rhs)); }
		NODISCARD constexpr f32 Length() const noexcept                                                      { return DirectX::XMVectorGetX(Ops::Length(*this)); }
		NODISCARD constexpr f32 LengthSquared() const noexcept                                               { return DirectX::XMVectorGetX(Ops::LengthSquared(*this)); }
		NODISCARD constexpr f32 Distance(const VectorBase& other) const noexcept                             { return DirectX::XMVectorGetX(Ops::Distance(*this, other)); }
		NODISCARD constexpr f32 DistanceSquared(const VectorBase& other) const noexcept                      { return DirectX::XMVectorGetX(Ops::DistanceSquared(*this, other)); }
		NODISCARD constexpr VectorBase Reflect(const VectorBase& normal) const noexcept                      { return Ops::Reflect(*this, normal); }
		NODISCARD constexpr VectorBase Refract(const VectorBase& normal, f32 refractionIndex) const noexcept { return Ops::Refract(*this, normal, refractionIndex); }
		NODISCARD constexpr VectorBase Lerp(const VectorBase& other, f32 t) const noexcept                   { return Ops::Lerp(*this, other, t); }
		NODISCARD void Clamp(f32 min = 0.0f, f32 max = 0.1f) noexcept                                        { *this = Ops::Clamp(*this, min, max); }

		NODISCARD constexpr VectorBase SmoothStep(const VectorBase& other, f32 t) const noexcept
		{
			t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;  // Clamp between 0 and 1
			t = t * t * (3.0f - 2.0f * t);

			return Lerp(other, t);
		}

		template<typename = std::enable_if_t<N <= 3>>
		NODISCARD constexpr VectorBase Cross(const VectorBase& rhs) const noexcept { return Ops::Cross(*this, rhs); }
		
		template<typename = std::enable_if_t<N == 4>>
		NODISCARD constexpr VectorBase Cross(const VectorBase& v1, const VectorBase& v2) noexcept { return Ops::Cross(*this, v1, v2); }

		static constexpr inline VectorBase Zero() noexcept
		{
			VectorBase result;
			std::fill_n(&result.x, N, Type(0));
			return result;
		}

		static constexpr inline VectorBase One() noexcept
		{
			VectorBase result;
			std::fill_n(&result.x, N, Type(1));
			return result;
		}

		static constexpr inline VectorBase UnitX() noexcept
		{
			VectorBase result = Zero();
			result.x = Type(1);
			return result;
		}

		static constexpr inline VectorBase UnitY() noexcept
		{
			VectorBase result = Zero();
			result.y = Type(1);
			return result;
		}

		template<typename = std::enable_if_t<(N >= 3)>>
		static constexpr inline VectorBase UnitZ() noexcept
		{
			VectorBase result = Zero();
			result.z = Type(1);
			return result;
		}

		static constexpr inline VectorBase Right() noexcept { return UnitX(); }
		
		static constexpr inline VectorBase Left() noexcept 
		{
			auto result = UnitX();
			result.x = Type(-1);
			return result;
		}

		static constexpr inline VectorBase Up() noexcept { return UnitY(); }
		static constexpr inline VectorBase Down() noexcept 
		{
			auto result = UnitY();
			result.y = Type(-1);
			return result;
		}

		template<typename = std::enable_if_t<(N >= 3)>>
		static constexpr inline VectorBase Forward() noexcept
		{
			VectorBase result = Zero();
			result.z = Type(-1);
			return result;
		}

		template<typename = std::enable_if_t<(N >= 3)>>
		static constexpr inline VectorBase Backward() noexcept
		{
			VectorBase result = Zero();
			result.z = Type(1);
			return result;
		}

		template<typename = std::enable_if_t<(N == 2)>>
		static constexpr inline VectorBase Forward2D() noexcept { return Up(); }

		template<typename = std::enable_if_t<(N == 2)>>
		static constexpr inline VectorBase Backward2D() noexcept { return Down(); }
	};
}


namespace Elos
{
	using Vector2f = Internal::VectorBase<f32, 2>;  // 2 dimensional floating point vector
	using Vector3f = Internal::VectorBase<f32, 3>;  // 3 dimensional floating point vector
	using Vector4f = Internal::VectorBase<f32, 4>;  // 4 dimensional floating point vector
	using Vector2i = Internal::VectorBase<i32, 2>;  // 2 dimensional integer vector
	using Vector3i = Internal::VectorBase<i32, 3>;  // 3 dimensional integer vector
	using Vector4i = Internal::VectorBase<i32, 4>;  // 4 dimensional integer vector
	using Vector2u = Internal::VectorBase<u32, 2>;  // 2 dimensional unsigned integer vector
	using Vector3u = Internal::VectorBase<u32, 3>;  // 3 dimensional unsigned integer vector
	using Vector4u = Internal::VectorBase<u32, 4>;  // 4 dimensional unsigned integer vector
}
