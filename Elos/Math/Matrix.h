#pragma once

#include <Elos/Math/MathBase.h>
#include <Elos/Math/Vector.h>

namespace Elos
{
	class Matrix : public DirectX::XMFLOAT4X4
	{
	public:
		using XMFLOAT4X4::XMFLOAT4X4;

		Matrix() noexcept;
		Matrix(DirectX::XMMATRIX mat) noexcept;

		operator DirectX::XMMATRIX() const noexcept;

		Matrix operator+(const Matrix& other) const noexcept;
		Matrix operator-(const Matrix& other) const noexcept;
		Matrix operator*(const Matrix& other) const noexcept;
		Matrix operator*(f32 scalar) const noexcept;
		Matrix operator/(const Matrix& other) const noexcept;
		Matrix operator/(f32 scalar) const noexcept;
		
		Matrix& operator+=(const Matrix& other) noexcept;
		Matrix& operator-=(const Matrix& other) noexcept;
		Matrix& operator*=(const Matrix& other) noexcept;
		Matrix& operator*=(f32 scalar) noexcept;
		Matrix& operator/=(const Matrix& other) noexcept;
		Matrix& operator/=(f32 scalar) noexcept;

		Matrix operator- () const noexcept;

		Vector3f Up() const noexcept;
		Vector3f Down() const noexcept;
		Vector3f Left() const noexcept;
		Vector3f Right() const noexcept;
		Vector3f Forward() const noexcept;
		Vector3f Backward() const noexcept;
		Vector3f Translation() const noexcept;
		bool Decompose(Vector3f outScale, Quaternion& outRotation, Vector3f& outTranslation) const noexcept;
		Matrix Transpose() const noexcept;
		Matrix Invert() const noexcept;
		f32 Determinant() const noexcept;
		Vector3f ToEuler() const noexcept;

		static Matrix CreateBillboard(const Vector3f& target, const Vector3f& camPos, const Vector3f& camUp, const Vector3f* camForward = nullptr) noexcept;
		static Matrix CreateConstraintBillboard(const Vector3f& target, const Vector3f& camPos, const Vector3f& rotateAxis, const Vector3f* camForward = nullptr, const Vector3f* targetForward = nullptr);
		static Matrix CreateTranslation(const Vector3f& position) noexcept;
		static Matrix CreateScale(const Vector3f scale);
		static Matrix CreateRotationX(f32 radians) noexcept;
		static Matrix CreateRotationY(f32 radians) noexcept;
		static Matrix CreateRotationZ(f32 radians) noexcept;
		static Matrix CreateFromAxisAngle(const Vector3f axis, f32 angle) noexcept;
		static Matrix CreatePerspectiveFOV(f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane) noexcept;
		static Matrix CreatePerspective(f32 width, f32 height, f32 nearPlane, f32 farPlane) noexcept;
		static Matrix CreatePerspectiveOffCentre(f32 left, f32 right, f32 top, f32 bottom, f32 nearPlane, f32 farPlane) noexcept;
		static Matrix CreateOrthographic(f32 width, f32 height, f32 nearPlaneZ, f32 farPlaneZ) noexcept;
		static Matrix CreateOrthographicOffCentre(f32 left, f32 right, f32 top, f32 bottom, f32 nearPlaneZ, f32 farPlaneZ) noexcept;
		static Matrix CreateLookAt(const Vector3f& position, const Vector3f& target, const Vector3f& up) noexcept;
		static Matrix CreateWorld(const Vector3f& position, const Vector3f& forward, const Vector3f& up) noexcept;
		static Matrix CreateFromQuaternion(const Quaternion& quat) noexcept;
		static Matrix CreateFromYawPitchRoll(const Vector3f angles) noexcept;  // Yaw: Y | Pitch: X | Roll: Z
		static Matrix Lerp(const Matrix& m1, const Matrix& m2, f32 t);
		static Matrix Transform(const Matrix& mat, const Quaternion& rotation);
		static Matrix Identity();
	};
}