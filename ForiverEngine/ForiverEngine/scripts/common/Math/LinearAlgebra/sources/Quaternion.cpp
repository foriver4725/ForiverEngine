#include "../headers/Quaternion.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Vector3.h"

namespace ForiverEngine
{
	Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angleRad) noexcept
	{
		const Vector3 normedAxis = axis.Normed();
		const float cos = std::cos(angleRad * 0.5f);
		const float sin = std::sin(angleRad * 0.5f);

		return Quaternion(
			normedAxis.x * sin,
			normedAxis.y * sin,
			normedAxis.z * sin,
			cos
		);
	}
	Quaternion Quaternion::VectorToVector(const Vector3& from, const Vector3& to) noexcept
	{
		const Vector3 f = from.Normed();
		const Vector3 t = to.Normed();
		const float dot = Vector3::Dot(f, t);
		if (dot >= 1.0f - Epsilon)
		{
			// 同じ方向を向いている場合
			return Quaternion::Identity();
		}
		else if (dot <= -1.0f + Epsilon)
		{
			// 逆方向を向いている場合
			Vector3 orthoAxis;
			if (std::abs(f.x) < std::abs(f.y) && std::abs(f.x) < std::abs(f.z))
				orthoAxis = Vector3(1.0f, 0.0f, 0.0f);
			else if (std::abs(f.y) < std::abs(f.z))
				orthoAxis = Vector3(0.0f, 1.0f, 0.0f);
			else
				orthoAxis = Vector3(0.0f, 0.0f, 1.0f);
			Vector3 axis = Vector3::Cross(f, orthoAxis).Normed();
			return FromAxisAngle(axis, 3.14159265f); // 180度回転
		}
		else
		{
			Vector3 axis = Vector3::Cross(f, t);
			float angle = std::acos(dot);
			return FromAxisAngle(axis, angle);
		}
	}

	Quaternion& Quaternion::operator=(const Quaternion& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
		}
		return *this;
	}
	Quaternion& Quaternion::operator=(Quaternion&& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
		}
		return *this;
	}

	bool Quaternion::operator==(const Quaternion& other) const noexcept
	{
		return std::abs(x - other.x) < Epsilon
			&& std::abs(y - other.y) < Epsilon
			&& std::abs(z - other.z) < Epsilon
			&& std::abs(w - other.w) < Epsilon;
	}
	bool Quaternion::operator!=(const Quaternion& other) const noexcept
	{
		return !(*this == other);
	}

	Quaternion Quaternion::operator*(const Quaternion& other) const noexcept
	{
		return Quaternion(
			w * other.x + x * other.w + y * other.z - z * other.y,
			w * other.y - x * other.z + y * other.w + z * other.x,
			w * other.z + x * other.y - y * other.x + z * other.w,
			w * other.w - x * other.x - y * other.y - z * other.z
		);
	}
	Vector3 Quaternion::operator*(const Vector3& vec) const noexcept
	{
		// q * v * q^-1
		Quaternion qVec(vec.x, vec.y, vec.z, 0.0f);
		Quaternion qConj(-x, -y, -z, w);
		Quaternion result = (*this * qVec) * qConj;
		return Vector3(result.x, result.y, result.z);
	}

	Quaternion& Quaternion::operator*=(const Quaternion& other) noexcept
	{
		x = w * other.x + x * other.w + y * other.z - z * other.y;
		y = w * other.y - x * other.z + y * other.w + z * other.x;
		z = w * other.z + x * other.y - y * other.x + z * other.w;
		w = w * other.w - x * other.x - y * other.y - z * other.z;

		return *this;
	}

	float Quaternion::LenSq() const noexcept
	{
		return x * x + y * y + z * z + w * w;
	}
	float Quaternion::Len() const noexcept
	{
		return std::sqrt(LenSq());
	}

	Quaternion Quaternion::Normalized() const noexcept
	{
		const float len = Len();
		if (std::abs(len) < Epsilon)
		{
			return Quaternion::Identity();
		}
		return Quaternion(x / len, y / len, z / len, w / len);
	}
	Quaternion& Quaternion::Norm() noexcept
	{
		*this = Normalized();
		return *this;
	}

	Quaternion Quaternion::Conjugate() const noexcept
	{
		return Quaternion(-x, -y, -z, w);
	}

	Quaternion Quaternion::Lerp(const Quaternion& from, const Quaternion& to, float t) noexcept
	{
		const float _t = std::clamp(t, 0.0f, 1.0f);
		return Quaternion(
			from.x + (to.x - from.x) * _t,
			from.y + (to.y - from.y) * _t,
			from.z + (to.z - from.z) * _t,
			from.w + (to.w - from.w) * _t
		).Normalized();
	}
	Quaternion Quaternion::Slerp(const Quaternion& from, const Quaternion& to, float t) noexcept
	{
		const float _t = std::clamp(t, 0.0f, 1.0f);
		float dot = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
		Quaternion to1 = to;
		if (dot < 0.0f)
		{
			dot = -dot;
			to1 = Quaternion(-to.x, -to.y, -to.z, -to.w);
		}
		if (dot > 0.9995f)
		{
			return Lerp(from, to1, _t);
		}
		const float theta_0 = std::acos(dot);
		const float theta = theta_0 * _t;
		const float sin_theta = std::sin(theta);
		const float sin_theta_0 = std::sin(theta_0);
		const float s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
		const float s1 = sin_theta / sin_theta_0;
		return Quaternion(
			(from.x * s0) + (to1.x * s1),
			(from.y * s0) + (to1.y * s1),
			(from.z * s0) + (to1.z * s1),
			(from.w * s0) + (to1.w * s1)
		).Normalized();
	}
}
