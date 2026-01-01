#include "../headers/Vector2.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Lattice2.h"

namespace ForiverEngine
{
	Vector2::Vector2(const Lattice2& lattice) noexcept : x(static_cast<float>(lattice.x)), y(static_cast<float>(lattice.y)) {}
	Vector2::Vector2(Lattice2&& lattice) noexcept : x(static_cast<float>(lattice.x)), y(static_cast<float>(lattice.y)) {}

	Vector2& Vector2::operator=(const Vector2& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
		}
		return *this;
	}
	Vector2& Vector2::operator=(Vector2&& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
		}
		return *this;
	}

	bool Vector2::operator==(const Vector2& other) const noexcept
	{
		return std::abs(x - other.x) < Epsilon
			&& std::abs(y - other.y) < Epsilon;
	}
	bool Vector2::operator!=(const Vector2& other) const noexcept
	{
		return !(*this == other);
	}

	Vector2 Vector2::operator+() const noexcept
	{
		return *this;
	}
	Vector2 Vector2::operator-() const noexcept
	{
		return Vector2(-x, -y);
	}

	Vector2 Vector2::operator+(const Vector2& other) const noexcept
	{
		return Vector2(x + other.x, y + other.y);
	}
	Vector2 Vector2::operator-(const Vector2& other) const noexcept
	{
		return Vector2(x - other.x, y - other.y);
	}
	Vector2 Vector2::operator*(float scalar) const noexcept
	{
		return Vector2(x * scalar, y * scalar);
	}
	Vector2 Vector2::operator/(float scalar) const noexcept
	{
		if (std::abs(scalar) < Epsilon)
		{
			return Vector2(0.0f, 0.0f);
		}
		return Vector2(x / scalar, y / scalar);
	}
	Vector2 operator*(float scalar, const Vector2& vec) noexcept
	{
		return Vector2(vec.x * scalar, vec.y * scalar);
	}

	Vector2& Vector2::operator+=(const Vector2& other) noexcept
	{
		x += other.x;
		y += other.y;

		return *this;
	}
	Vector2& Vector2::operator-=(const Vector2& other) noexcept
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}
	Vector2& Vector2::operator*=(float scalar) noexcept
	{
		x *= scalar;
		y *= scalar;

		return *this;
	}
	Vector2& Vector2::operator/=(float scalar) noexcept
	{
		if (std::abs(scalar) < Epsilon)
		{
			x = 0.0f;
			y = 0.0f;
		}
		else
		{
			x /= scalar;
			y /= scalar;
		}

		return *this;
	}

	float Vector2::LenSq() const noexcept
	{
		return x * x + y * y;
	}
	float Vector2::Len() const noexcept
	{
		return std::sqrt(LenSq());
	}

	Vector2 Vector2::Normed() const noexcept
	{
		const float len = Len();
		if (std::abs(len) < Epsilon)
		{
			return Vector2(0.0f, 0.0f);
		}
		return *this / len;
	}
	Vector2& Vector2::Norm() noexcept
	{
		*this = Normed();
		return *this;
	}

	float Vector2::Dot(const Vector2& lhs, const Vector2& rhs) noexcept
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	float Vector2::Cross(const Vector2& lhs, const Vector2& rhs) noexcept
	{
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	Vector2 Vector2::Lerp(const Vector2& from, const Vector2& to, float t) noexcept
	{
		const float _t = std::clamp(t, 0.0f, 1.0f);
		return from + (to - from) * _t;
	}
	Vector2 Vector2::Slerp(const Vector2& from, const Vector2& to, float t) noexcept
	{
		const float _t = std::clamp(t, 0.0f, 1.0f);
		const float dot = Dot(from.Normed(), to.Normed());
		const float clampedDot = std::clamp(dot, -1.0f, 1.0f);
		const float theta = std::acos(clampedDot) * _t;
		const Vector2 relativeVec = (to - from * dot).Normed();
		return (from * std::cos(theta)) + (relativeVec * std::sin(theta));
	}
	Vector2 Vector2::Reflect(const Vector2& vec, const Vector2& normal) noexcept
	{
		const Vector2 _normal = normal.Normed();
		const float dot = Dot(vec, _normal);
		return vec - 2.0f * dot * _normal;
	}
}
