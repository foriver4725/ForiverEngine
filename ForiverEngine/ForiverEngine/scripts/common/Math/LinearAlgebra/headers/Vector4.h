#pragma once

namespace ForiverEngine
{
	struct Lattice4;
	struct Vector3;

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;

		constexpr Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

		constexpr Vector4(float x, float y, float z, float w = 1.0f) noexcept : x(x), y(y), z(z), w(w) {}
		constexpr Vector4(const Vector4& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}
		constexpr Vector4(Vector4&& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}

		constexpr Vector4(int x, int y, int z, int w = 1) noexcept : x(static_cast<float>(x)), y(static_cast<float>(y)), z(static_cast<float>(z)), w(static_cast<float>(w)) {}
		Vector4(const Lattice4& lattice) noexcept;
		Vector4(Lattice4&& lattice) noexcept;

		Vector4(const Vector3& vec, float w = 1.0f) noexcept;
		Vector4(Vector3&& vec, float w = 1.0f) noexcept;

		Vector4& operator=(const Vector4& other) noexcept;
		Vector4& operator=(Vector4&& other) noexcept;

		bool operator==(const Vector4& other) const noexcept;
		bool operator!=(const Vector4& other) const noexcept;

		Vector4 operator+() const noexcept;
		Vector4 operator-() const noexcept;

		Vector4 operator+(const Vector4& other) const noexcept;
		Vector4 operator-(const Vector4& other) const noexcept;
		Vector4 operator*(float scalar) const noexcept;
		Vector4 operator/(float scalar) const noexcept;
		friend Vector4 operator*(float scalar, const Vector4& vec) noexcept;

		Vector4& operator+=(const Vector4& other) noexcept;
		Vector4& operator-=(const Vector4& other) noexcept;
		Vector4& operator*=(float scalar) noexcept;
		Vector4& operator/=(float scalar) noexcept;

		float LenSq() const noexcept;
		float Len() const noexcept;

		Vector4 Normed() const noexcept;
		Vector4& Norm() noexcept;

		static float Dot(const Vector4& a, const Vector4& b) noexcept;

		static Vector4 Lerp(const Vector4& a, const Vector4& b, float t) noexcept;
		static Vector4 Slerp(const Vector4& a, const Vector4& b, float t) noexcept;
	};
}