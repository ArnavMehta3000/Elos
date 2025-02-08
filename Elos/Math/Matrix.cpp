#include <Elos/Math/Matrix.h>
#include <Elos/Math/Quaternion.h>
#include <cassert>

namespace Elos
{
	Matrix::Matrix() noexcept : XMFLOAT4X4()
	{
		DirectX::XMStoreFloat4x4(this, DirectX::XMMatrixIdentity());
	}

	Matrix::Matrix(DirectX::XMMATRIX mat) noexcept
	{
		DirectX::XMStoreFloat4x4(this, mat);
	}

	Matrix::operator DirectX::XMMATRIX() const noexcept
	{
		return DirectX::XMLoadFloat4x4(this);
	}

	Matrix Matrix::operator+(const Matrix& other) const noexcept
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_41));

		const DirectX::XMVECTOR y1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._11));
		const DirectX::XMVECTOR y2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._21));
		const DirectX::XMVECTOR y3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._31));
		const DirectX::XMVECTOR y4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._41));

		x1 = DirectX::XMVectorAdd(x1, y1);
		x2 = DirectX::XMVectorAdd(x2, y2);
		x3 = DirectX::XMVectorAdd(x3, y3);
		x4 = DirectX::XMVectorAdd(x4, y4);

		Matrix result;
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._41), x4);
		return result;
	}

	Matrix Matrix::operator-(const Matrix& other) const noexcept
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_41));

		const DirectX::XMVECTOR y1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._11));
		const DirectX::XMVECTOR y2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._21));
		const DirectX::XMVECTOR y3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._31));
		const DirectX::XMVECTOR y4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._41));

		x1 = DirectX::XMVectorSubtract(x1, y1);
		x2 = DirectX::XMVectorSubtract(x2, y2);
		x3 = DirectX::XMVectorSubtract(x3, y3);
		x4 = DirectX::XMVectorSubtract(x4, y4);

		Matrix result;
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._41), x4);
		return result;
	}

	Matrix Matrix::operator*(const Matrix& other) const noexcept
	{
		const DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(this);
		const DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(&other);
		const DirectX::XMMATRIX X = DirectX::XMMatrixMultiply(m1, m2);

		Matrix result;
		DirectX::XMStoreFloat4x4(&result, X);
		return result;
	}

	Matrix Matrix::operator*(f32 scalar) const noexcept
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_41));

		x1 = DirectX::XMVectorScale(x1, scalar);
		x2 = DirectX::XMVectorScale(x2, scalar);
		x3 = DirectX::XMVectorScale(x3, scalar);
		x4 = DirectX::XMVectorScale(x4, scalar);

		Matrix result;
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._41), x4);
		return result;
	}

	Matrix Matrix::operator/(const Matrix& other) const noexcept
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_41));

		const DirectX::XMVECTOR y1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._11));
		const DirectX::XMVECTOR y2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._21));
		const DirectX::XMVECTOR y3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._31));
		const DirectX::XMVECTOR y4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._41));

		x1 = DirectX::XMVectorDivide(x1, y1);
		x2 = DirectX::XMVectorDivide(x2, y2);
		x3 = DirectX::XMVectorDivide(x3, y3);
		x4 = DirectX::XMVectorDivide(x4, y4);

		Matrix result;
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._41), x4);
		return result;
	}

	Matrix Matrix::operator/(f32 scalar) const noexcept
	{
		assert(scalar != 0.f);

		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&this->_41));

		const float rs = 1.f / scalar;

		x1 = DirectX::XMVectorScale(x1, rs);
		x2 = DirectX::XMVectorScale(x2, rs);
		x3 = DirectX::XMVectorScale(x3, rs);
		x4 = DirectX::XMVectorScale(x4, rs);

		Matrix result;
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._41), x4);
		return result;
	}

	Matrix& Matrix::operator+=(const Matrix& other) noexcept
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_41));

		const DirectX::XMVECTOR y1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._11));
		const DirectX::XMVECTOR y2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._21));
		const DirectX::XMVECTOR y3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._31));
		const DirectX::XMVECTOR y4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._41));

		x1 = DirectX::XMVectorAdd(x1, y1);
		x2 = DirectX::XMVectorAdd(x2, y2);
		x3 = DirectX::XMVectorAdd(x3, y3);
		x4 = DirectX::XMVectorAdd(x4, y4);

		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_41), x4);
		return *this;
	}

	Matrix& Matrix::operator-=(const Matrix& other) noexcept
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_41));

		const DirectX::XMVECTOR y1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._11));
		const DirectX::XMVECTOR y2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._21));
		const DirectX::XMVECTOR y3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._31));
		const DirectX::XMVECTOR y4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._41));

		x1 = DirectX::XMVectorSubtract(x1, y1);
		x2 = DirectX::XMVectorSubtract(x2, y2);
		x3 = DirectX::XMVectorSubtract(x3, y3);
		x4 = DirectX::XMVectorSubtract(x4, y4);

		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_41), x4);
		return *this;
	}

	Matrix& Matrix::operator*=(const Matrix& other) noexcept
	{
		const DirectX::XMMATRIX M1 = *this;
		const DirectX::XMMATRIX M2 = other;
		const DirectX::XMMATRIX X = DirectX::XMMatrixMultiply(M1, M2);
		DirectX::XMStoreFloat4x4(this, X);
		return *this;
	}

	Matrix& Matrix::operator*=(f32 scalar) noexcept
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_41));

		x1 = DirectX::XMVectorScale(x1, scalar);
		x2 = DirectX::XMVectorScale(x2, scalar);
		x3 = DirectX::XMVectorScale(x3, scalar);
		x4 = DirectX::XMVectorScale(x4, scalar);

		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_41), x4);
		return *this;
	}

	Matrix& Matrix::operator/=(const Matrix& other) noexcept
	{
		DirectX::XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_11));
		DirectX::XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_21));
		DirectX::XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_31));
		DirectX::XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_41));

		const DirectX::XMVECTOR y1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._11));
		const DirectX::XMVECTOR y2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._21));
		const DirectX::XMVECTOR y3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._31));
		const DirectX::XMVECTOR y4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&other._41));

		x1 = DirectX::XMVectorDivide(x1, y1);
		x2 = DirectX::XMVectorDivide(x2, y2);
		x3 = DirectX::XMVectorDivide(x3, y3);
		x4 = DirectX::XMVectorDivide(x4, y4);

		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_41), x4);
		return *this;
	}

	Matrix& Matrix::operator/=(f32 scalar) noexcept
	{
		assert(scalar != 0.f);
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_41));

		const float rs = 1.f / scalar;

		x1 = DirectX::XMVectorScale(x1, rs);
		x2 = DirectX::XMVectorScale(x2, rs);
		x3 = DirectX::XMVectorScale(x3, rs);
		x4 = DirectX::XMVectorScale(x4, rs);

		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&_41), x4);
		return *this;
	}

	Matrix Matrix::operator-() const noexcept
	{
		DirectX::XMVECTOR v1 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_11));
		DirectX::XMVECTOR v2 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_21));
		DirectX::XMVECTOR v3 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_31));
		DirectX::XMVECTOR v4 = XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(&_41));

		v1 = DirectX::XMVectorNegate(v1);
		v2 = DirectX::XMVectorNegate(v2);
		v3 = DirectX::XMVectorNegate(v3);
		v4 = DirectX::XMVectorNegate(v4);

		Matrix R;
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&R._11), v1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&R._21), v2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&R._31), v3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&R._41), v4);
		return R;
	}

	Vector3f Matrix::Up() const noexcept
	{
		return Vector3f(_21, _22, _23);
	}

	Vector3f Matrix::Down() const noexcept
	{
		return Vector3f(-_21, -_22, -_23);
	}

	Vector3f Matrix::Left() const noexcept
	{
		return Vector3f(-_11, -_12, -_13);
	}

	Vector3f Matrix::Right() const noexcept
	{
		return Vector3f(_11, _12, _13);
	}

	Vector3f Matrix::Forward() const noexcept
	{
		return Vector3f(-_31, -_32, -_33);
	}

	Vector3f Matrix::Backward() const noexcept
	{
		return Vector3f(_31, _32, _33);
	}

	Vector3f Matrix::Translation() const noexcept
	{
		return Vector3f(_41, _42, _43);
	}

	bool Matrix::Decompose(Vector3f outScale, Quaternion& outRotation, Vector3f& outTranslation) const noexcept
	{
		DirectX::XMVECTOR s, r, t;

		if (!DirectX::XMMatrixDecompose(&s, &r, &t, *this))
		{
			return false;
		}

		DirectX::XMStoreFloat3(&outScale, s);
		DirectX::XMStoreFloat4(&outRotation, r);
		DirectX::XMStoreFloat3(&outTranslation, t);

		return true;
	}

	Matrix Matrix::Transpose() const noexcept
	{
		Matrix result;
		DirectX::XMStoreFloat4x4(&result, *this);
		return result;
	}

	Matrix Matrix::Invert() const noexcept
	{
		Matrix result;
		DirectX::XMVECTOR det;
		DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixInverse(&det, *this));
		return result;
	}

	f32 Matrix::Determinant() const noexcept
	{
		return DirectX::XMVectorGetX(DirectX::XMMatrixDeterminant(*this));
	}

	Vector3f Matrix::ToEuler() const noexcept
	{
		const f32 cy = sqrtf(_33 * _33 + _31 * _31);
		const f32 cx = atan2f(-_32, cy);
		if (cy > 16.f * FLT_EPSILON)
		{
			return Vector3f(cx, atan2f(_31, _33), atan2f(_12, _22));
		}
		else
		{
			return Vector3f(cx, 0.f, atan2f(-_21, _11));
		}
	}

	Matrix Matrix::CreateBillboard(const Vector3f& target, const Vector3f& camPos, const Vector3f& camUp, const Vector3f* camForward) noexcept
	{
		using namespace DirectX;
		const XMVECTOR T = target;
		const XMVECTOR C = camPos;
		XMVECTOR Z = T - C;

		const XMVECTOR N = XMVector3LengthSq(Z);
		if (XMVector3Less(N, g_XMEpsilon))
		{
			if (camForward)
			{
				const XMVECTOR F = *camForward;
				Z = -F;
			}
			else
			{
				Z = g_XMNegIdentityR2;
			}
		}
		else
		{
			Z = XMVector3Normalize(Z);
		}

		const XMVECTOR up = camUp;
		XMVECTOR X = XMVector3Cross(up, Z);
		X = XMVector3Normalize(X);

		const XMVECTOR Y = XMVector3Cross(Z, X);

		XMMATRIX M;
		M.r[0] = X;
		M.r[1] = Y;
		M.r[2] = Z;
		M.r[3] = XMVectorSetW(T, 1.f);

		Matrix result;
		XMStoreFloat4x4(&result, M);
		return result;
	}

	Matrix Matrix::CreateConstraintBillboard(const Vector3f& target, const Vector3f& camPos, const Vector3f& rotateAxis, const Vector3f* camForward, const Vector3f* targetForward)
	{
		using namespace DirectX;

		static const XMVECTORF32 s_minAngle = { { { 0.99825467075f, 0.99825467075f, 0.99825467075f, 0.99825467075f } } }; // 1.0 - XMConvertToRadians( 0.1f );

		const XMVECTOR T = target;
		const XMVECTOR C = camPos;
		XMVECTOR faceDir = T - C;

		const XMVECTOR N = XMVector3LengthSq(faceDir);
		if (XMVector3Less(N, g_XMEpsilon))
		{
			if (camForward)
			{
				const XMVECTOR F = *camForward;
				faceDir = XMVectorNegate(F);
			}
			else
				faceDir = g_XMNegIdentityR2;
		}
		else
		{
			faceDir = XMVector3Normalize(faceDir);
		}

		const XMVECTOR Y = rotateAxis;
		XMVECTOR X, Z;

		XMVECTOR dot = XMVectorAbs(XMVector3Dot(Y, faceDir));
		if (XMVector3Greater(dot, s_minAngle))
		{
			if (targetForward)
			{
				Z = *targetForward;
				dot = XMVectorAbs(XMVector3Dot(Y, Z));
				if (XMVector3Greater(dot, s_minAngle))
				{
					dot = XMVectorAbs(XMVector3Dot(Y, g_XMNegIdentityR2));
					Z = (XMVector3Greater(dot, s_minAngle)) ? g_XMIdentityR0 : g_XMNegIdentityR2;
				}
			}
			else
			{
				dot = XMVectorAbs(XMVector3Dot(Y, g_XMNegIdentityR2));
				Z = (XMVector3Greater(dot, s_minAngle)) ? g_XMIdentityR0 : g_XMNegIdentityR2;
			}

			X = XMVector3Cross(Y, Z);
			X = XMVector3Normalize(X);

			Z = XMVector3Cross(X, Y);
			Z = XMVector3Normalize(Z);
		}
		else
		{
			X = XMVector3Cross(Y, faceDir);
			X = XMVector3Normalize(X);

			Z = XMVector3Cross(X, Y);
			Z = XMVector3Normalize(Z);
		}

		XMMATRIX M;
		M.r[0] = X;
		M.r[1] = Y;
		M.r[2] = Z;
		M.r[3] = XMVectorSetW(T, 1.f);

		Matrix R;
		XMStoreFloat4x4(&R, M);
		return R;
	}

	Matrix Matrix::CreateTranslation(const Vector3f& position) noexcept
	{
		return DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	}

	Matrix Matrix::CreateScale(const Vector3f scale)
	{
		return DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	}

	Matrix Matrix::CreateRotationX(f32 radians) noexcept
	{
		return DirectX::XMMatrixRotationX(radians);
	}

	Matrix Matrix::CreateRotationY(f32 radians) noexcept
	{
		return DirectX::XMMatrixRotationY(radians);
	}

	Matrix Matrix::CreateRotationZ(f32 radians) noexcept
	{
		return DirectX::XMMatrixRotationZ(radians);
	}

	Matrix Matrix::CreateFromAxisAngle(const Vector3f axis, f32 angle) noexcept
	{
		return DirectX::XMMatrixRotationAxis(axis, angle);
	}

	Matrix Matrix::CreatePerspectiveFOV(f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane) noexcept
	{
		return DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane);
	}

	Matrix Matrix::CreatePerspective(f32 width, f32 height, f32 nearPlane, f32 farPlane) noexcept
	{
		return DirectX::XMMatrixPerspectiveRH(width, height, nearPlane, farPlane);
	}

	Matrix Matrix::CreatePerspectiveOffCentre(f32 left, f32 right, f32 top, f32 bottom, f32 nearPlane, f32 farPlane) noexcept
	{
		return DirectX::XMMatrixPerspectiveOffCenterRH(left, right, bottom, top, nearPlane, farPlane);
	}

	Matrix Matrix::CreateOrthographic(f32 width, f32 height, f32 nearPlaneZ, f32 farPlaneZ) noexcept
	{
		return DirectX::XMMatrixOrthographicRH(width, height, nearPlaneZ, farPlaneZ);
	}

	Matrix Matrix::CreateOrthographicOffCentre(f32 left, f32 right, f32 top, f32 bottom, f32 nearPlaneZ, f32 farPlaneZ) noexcept
	{
		return DirectX::XMMatrixOrthographicOffCenterRH(left, right, bottom, top, nearPlaneZ, farPlaneZ) :
	}

	Matrix Matrix::CreateLookAt(const Vector3f& position, const Vector3f& target, const Vector3f& up) noexcept
	{
		return DirectX::XMMatrixLookAtRH(position, target, up);
	}

	Matrix Matrix::CreateWorld(const Vector3f& position, const Vector3f& forward, const Vector3f& up) noexcept
	{
		const DirectX::XMVECTOR zAxis = Vector3f::Ops::Normalize(-forward);
		const DirectX::XMVECTOR xAxis = Vector3f::Ops::Normalize(Vector3f::Ops::Cross(up, zAxis));
		const DirectX::XMVECTOR yAxis = Vector3f::Ops::Normalize(Vector3f::Ops::Cross(zAxis, xAxis));
		
		Matrix result;
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&result._11), xAxis);
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&result._21), yAxis);
		DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&result._31), zAxis);
		
		result._14 = result._24 = result._34 = 0.f;
		result._41 = position.x; result._42 = position.y; result._43 = position.z;
		result._44 = 1.f;
		
		return result;
	}

	Matrix Matrix::CreateFromQuaternion(const Quaternion& quat) noexcept
	{
		return DirectX::XMMatrixRotationQuaternion(quat);
	}

	Matrix Matrix::CreateFromYawPitchRoll(const Vector3f angles) noexcept
	{
		return DirectX::XMMatrixRotationRollPitchYaw(angles.x, angles.y, angles.z);
	}

	Matrix Matrix::Lerp(const Matrix& m1, const Matrix& m2, f32 t)
	{
		DirectX::XMVECTOR x1 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m1._11));
		DirectX::XMVECTOR x2 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m1._21));
		DirectX::XMVECTOR x3 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m1._31));
		DirectX::XMVECTOR x4 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m1._41));

		const DirectX::XMVECTOR y1 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m2._11));
		const DirectX::XMVECTOR y2 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m2._21));
		const DirectX::XMVECTOR y3 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m2._31));
		const DirectX::XMVECTOR y4 = DirectX::XMLoadFloat4(reinterpret_cast<const  DirectX::XMFLOAT4*>(&m2._41));

		x1 = DirectX::XMVectorLerp(x1, y1, t);
		x2 = DirectX::XMVectorLerp(x2, y2, t);
		x3 = DirectX::XMVectorLerp(x3, y3, t);
		x4 = DirectX::XMVectorLerp(x4, y4, t);

		Matrix result;
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._11), x1);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._21), x2);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._31), x3);
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result._41), x4);
		return result;
	}

	Matrix Matrix::Transform(const Matrix& mat, const Quaternion& rotation)
	{
		return DirectX::XMMatrixMultiply(mat, DirectX::XMMatrixRotationQuaternion(rotation));
	}

	Matrix Matrix::Identity()
	{
		return Matrix();
	}

}
