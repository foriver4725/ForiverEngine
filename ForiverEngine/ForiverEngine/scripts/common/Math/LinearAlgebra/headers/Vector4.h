#pragma once

namespace ForiverEngine
{
	struct Vector3;

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;

		constexpr Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
		constexpr Vector4(float x, float y, float z) noexcept : x(x), y(y), z(z), w(1.0f) {}
		constexpr Vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
		constexpr Vector4(const Vector4& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}
		constexpr Vector4(Vector4&& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}

		Vector4(const Vector3& v) noexcept;
		Vector4(const Vector3& v, float w) noexcept;

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