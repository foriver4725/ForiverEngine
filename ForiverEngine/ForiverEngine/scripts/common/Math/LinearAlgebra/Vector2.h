#pragma once

#include "./Defines.h"

namespace ForiverEngine
{
	struct Vector2
	{
		float x;
		float y;

		constexpr Vector2() noexcept : x(0.0f), y(0.0f) {}
		constexpr Vector2(float x, float y) noexcept : x(x), y(y) {}
		constexpr Vector2(const Vector2& other) noexcept : x(other.x), y(other.y) {}
		constexpr Vector2(Vector2&& other) noexcept : x(other.x), y(other.y) {}

		static constexpr Vector2 Zero() noexcept { return Vector2(0.0f, 0.0f); }
		static constexpr Vector2 One() noexcept { return Vector2(1.0f, 1.0f); }
		static constexpr Vector2 Right() noexcept { return Vector2(1.0f, 0.0f); }
		static constexpr Vector2 Left() noexcept { return Vector2(-1.0f, 0.0f); }
		static constexpr Vector2 Up() noexcept { return Vector2(0.0f, 1.0f); }
		static constexpr Vector2 Down() noexcept { return Vector2(0.0f, -1.0f); }

		Vector2& operator=(const Vector2& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
			}
			return *this;
		}
		Vector2& operator=(Vector2&& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
			}
			return *this;
		}

		bool operator==(const Vector2& other) const noexcept
		{
			return std::abs(x - other.x) < Epsilon
				&& std::abs(y - other.y) < Epsilon;
		}
		bool operator!=(const Vector2& other) const noexcept
		{
			return !(*this == other);
		}

		Vector2 operator+(const Vector2& other) const noexcept
		{
			return Vector2(x + other.x, y + other.y);
		}
		Vector2 operator-(const Vector2& other) const noexcept
		{
			return Vector2(x - other.x, y - other.y);
		}
		Vector2 operator*(float scalar) const noexcept
		{
			return Vector2(x * scalar, y * scalar);
		}
		Vector2 operator/(float scalar) const noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				return Vector2(0.0f, 0.0f);
			}
			return Vector2(x / scalar, y / scalar);
		}
		friend Vector2 operator*(float scalar, const Vector2& vec) noexcept
		{
			return Vector2(vec.x * scalar, vec.y * scalar);
		}

		void operator+=(const Vector2& other) noexcept
		{
			x += other.x;
			y += other.y;
		}
		void operator-=(const Vector2& other) noexcept
		{
			x -= other.x;
			y -= other.y;
		}
		void operator*=(float scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
		}
		void operator/=(float scalar) noexcept
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
		}

		float LenSq() const noexcept
		{
			return x * x + y * y;
		}
		float Len() const noexcept
		{
			return std::sqrt(LenSq());
		}

		Vector2 Normed() const noexcept
		{
			const float len = Len();
			if (std::abs(len) < Epsilon)
			{
				return Vector2(0.0f, 0.0f);
			}
			return *this / len;
		}
		void Norm() noexcept
		{
			*this = Normed();
		}

		static float Dot(const Vector2& lhs, const Vector2& rhs) noexcept
		{
			return lhs.x * rhs.x + lhs.y * rhs.y;
		}
		static float Cross(const Vector2& lhs, const Vector2& rhs) noexcept
		{
			return lhs.x * rhs.y - lhs.y * rhs.x;
		}

		static Vector2 Lerp(const Vector2& from, const Vector2& to, float t) noexcept
		{
			const float _t = std::clamp(t, 0.0f, 1.0f);
			return from + (to - from) * _t;
		}
		static Vector2 Reflect(const Vector2& vec, const Vector2& normal) noexcept
		{
			const Vector2 _normal = normal.Normed();
			const float dot = Dot(vec, _normal);
			return vec - 2.0f * dot * _normal;
		}
	};
}
