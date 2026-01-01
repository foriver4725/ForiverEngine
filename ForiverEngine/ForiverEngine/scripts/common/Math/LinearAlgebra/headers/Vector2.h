#pragma once

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

		Vector2& operator=(const Vector2& other) noexcept;
		Vector2& operator=(Vector2&& other) noexcept;

		bool operator==(const Vector2& other) const noexcept;
		bool operator!=(const Vector2& other) const noexcept;

		Vector2 operator+() const noexcept;
		Vector2 operator-() const noexcept;

		Vector2 operator+(const Vector2& other) const noexcept;
		Vector2 operator-(const Vector2& other) const noexcept;
		Vector2 operator*(float scalar) const noexcept;
		Vector2 operator/(float scalar) const noexcept;
		friend Vector2 operator*(float scalar, const Vector2& vec) noexcept;

		Vector2& operator+=(const Vector2& other) noexcept;
		Vector2& operator-=(const Vector2& other) noexcept;
		Vector2& operator*=(float scalar) noexcept;
		Vector2& operator/=(float scalar) noexcept;

		float LenSq() const noexcept;
		float Len() const noexcept;

		Vector2 Normed() const noexcept;
		Vector2& Norm() noexcept;

		static float Dot(const Vector2& lhs, const Vector2& rhs) noexcept;
		static float Cross(const Vector2& lhs, const Vector2& rhs) noexcept;

		static Vector2 Lerp(const Vector2& from, const Vector2& to, float t) noexcept;
		static Vector2 Slerp(const Vector2& from, const Vector2& to, float t) noexcept;
		static Vector2 Reflect(const Vector2& vec, const Vector2& normal) noexcept;
	};
}
