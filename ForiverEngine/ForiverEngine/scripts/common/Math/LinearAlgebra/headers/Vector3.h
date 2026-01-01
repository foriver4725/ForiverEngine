#pragma once

namespace ForiverEngine
{
	struct Lattice3;
	struct Vector2;

	struct Vector3
	{
		float x;
		float y;
		float z;

		constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
		constexpr Vector3(float x, float y, float z = 0.0f) noexcept : x(x), y(y), z(z) {}
		constexpr Vector3(const Vector3& other) noexcept : x(other.x), y(other.y), z(other.z) {}
		constexpr Vector3(Vector3&& other) noexcept : x(other.x), y(other.y), z(other.z) {}

		explicit Vector3(const Vector2& vec, float z = 0.0f) noexcept;
		explicit Vector3(const Lattice3& lattice) noexcept;

		static constexpr Vector3 Zero() noexcept { return Vector3(0.0f, 0.0f, 0.0f); }
		static constexpr Vector3 One() noexcept { return Vector3(1.0f, 1.0f, 1.0f); }
		static constexpr Vector3 Right() noexcept { return Vector3(1.0f, 0.0f, 0.0f); }
		static constexpr Vector3 Left() noexcept { return Vector3(-1.0f, 0.0f, 0.0f); }
		static constexpr Vector3 Up() noexcept { return Vector3(0.0f, 1.0f, 0.0f); }
		static constexpr Vector3 Down() noexcept { return Vector3(0.0f, -1.0f, 0.0f); }
		static constexpr Vector3 Forward() noexcept { return Vector3(0.0f, 0.0f, 1.0f); }
		static constexpr Vector3 Backward() noexcept { return Vector3(0.0f, 0.0f, -1.0f); }

		Vector3& operator=(const Vector3& other) noexcept;
		Vector3& operator=(Vector3&& other) noexcept;

		bool operator==(const Vector3& other) const noexcept;
		bool operator!=(const Vector3& other) const noexcept;

		Vector3 operator+() const noexcept;
		Vector3 operator-() const noexcept;

		Vector3 operator+(const Vector3& other) const noexcept;
		Vector3 operator-(const Vector3& other) const noexcept;
		Vector3 operator*(float scalar) const noexcept;
		Vector3 operator/(float scalar) const noexcept;
		friend Vector3 operator*(float scalar, const Vector3& vec) noexcept;

		Vector3& operator+=(const Vector3& other) noexcept;
		Vector3& operator-=(const Vector3& other) noexcept;
		Vector3& operator*=(float scalar) noexcept;
		Vector3& operator/=(float scalar) noexcept;

		float LenSq() const noexcept;
		float Len() const noexcept;

		Vector3 Normed() const noexcept;
		Vector3& Norm() noexcept;

		static float Dot(const Vector3& lhs, const Vector3& rhs) noexcept;
		static Vector3 Cross(const Vector3& lhs, const Vector3& rhs) noexcept;

		static Vector3 Lerp(const Vector3& from, const Vector3& to, float t) noexcept;
		static Vector3 Slerp(const Vector3& from, const Vector3& to, float t) noexcept;
		static Vector3 Reflect(const Vector3& vec, const Vector3& normal) noexcept;
	};
}