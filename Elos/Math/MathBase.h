#pragma once

#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/StandardTypes.h>
#include <DirectXMath.h>

namespace Elos
{
	constexpr f32 ELOS_PI      = DirectX::XM_PI;
	constexpr f32 ELOS_2PI     = DirectX::XM_2PI;
	constexpr f32 ELOS_1DIVPI  = DirectX::XM_1DIVPI;
	constexpr f32 ELOS_1DIV2PI = DirectX::XM_1DIV2PI;
	constexpr f32 ELOS_PIDIV2  = DirectX::XM_PIDIV2;
	constexpr f32 ELOS_PIDIV4  = DirectX::XM_PIDIV4;

	constexpr f32 ToRadians(f32 degrees) { return DirectX::XMConvertToRadians(degrees); }
	constexpr f32 ToDegrees(f32 radians) { return DirectX::XMConvertToDegrees(radians); }
}