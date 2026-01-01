#include "../headers/Vector3.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Vector2.h"

namespace ForiverEngine
{
	Vector3::Vector3(const Vector2& vec2) noexcept : x(vec2.x), y(vec2.y), z(0.0f) {}
	Vector3::Vector3(const Vector2& vec2, float z) noexcept : x(vec2.x), y(vec2.y), z(z) {}

	Vector3& Vector3::operator=(const Vector3& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}
		return *this;
	}
	Vector3& Vector3::operator=(Vector3&& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}
		return *this;
	}

	bool Vector3::operator==(const Vector3& other) const noexcept
	{
		return std::abs(x - other.x) < Epsilon
			&& std::abs(y - other.y) < Epsilon
			&& std::abs(z - other.z) < Epsilon;
	}
	bool Vector3::operator!=(const Vector3& other) const noexcept
	{
		return !(*this == other);
	}

	Vector3 Vector3::operator+() const noexcept
	{
		return *this;
	}
	Vector3 Vector3::operator-() const noexcept
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 Vector3::operator+(const Vector3& other) const noexcept
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	Vector3 Vector3::operator-(const Vector3& other) const noexcept
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	Vector3 Vector3::operator*(float scalar) const noexcept
	{
		return Vector3(x * scalar, y * scalar, z * scalar);
	}
	Vector3 Vector3::operator/(float scalar) const noexcept
	{
		if (std::abs(scalar) < Epsilon)
		{
			return Vector3(0.0f, 0.0f, 0.0f);
		}
		return Vector3(x / scalar, y / scalar, z / scalar);
	}
	Vector3 operator*(float scalar, const Vector3& vec) noexcept
	{
		return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
	}

	Vector3& Vector3::operator+=(const Vector3& other) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}
	Vector3& Vector3::operator-=(const Vector3& other) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}
	Vector3& Vector3::operator*=(float scalar) noexcept
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;

		return *this;
	}
	Vector3& Vector3::operator/=(float scalar) noexcept
	{
		if (std::abs(scalar) < Epsilon)
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
		else
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
		}

		return *this;
	}

	float Vector3::LenSq() const noexcept
	{
		return x * x + y * y + z * z;
	}
	float Vector3::Len() const noexcept
	{
		return std::sqrt(LenSq());
	}

	Vector3 Vector3::Normed() const noexcept
	{
		const float len = Len();
		if (std::abs(len) < Epsilon)
		{
			return Vector3(0.0f, 0.0f, 0.0f);
		}
		return *this / len;
	}
	Vector3& Vector3::Norm() noexcept
	{
		*this = Normed();
		return *this;
	}

	float Vector3::Dot(const Vector3& lhs, const Vector3& rhs) noexcept
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}
	Vector3 Vector3::Cross(const Vector3& lhs, const Vector3& rhs) noexcept
	{
		return Vector3(
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		);
	}

	Vector3 Vector3::Lerp(const Vector3& from, const Vector3& to, float t) noexcept
	{
		const float _t = std::clamp(t, 0.0f, 1.0f);
		return from + (to - from) * _t;
	}
	Vector3 Vector3::Slerp(const Vector3& from, const Vector3& to, float t) noexcept
	{
		const float _t = std::clamp(t, 0.0f, 1.0f);
		const float dot = Dot(from.Normed(), to.Normed());
		const float theta = std::acos(std::clamp(dot, -1.0f, 1.0f)) * _t;
		const Vector3 relativeVec = (to - from * dot).Normed();
		return (from * std::cos(theta)) + (relativeVec * std::sin(theta));
	}
	Vector3 Vector3::Reflect(const Vector3& vec, const Vector3& normal) noexcept
	{
		const Vector3 _normal = normal.Normed();
		const float dot = Dot(vec, _normal);
		return vec - 2.0f * dot * _normal;
	}
}
