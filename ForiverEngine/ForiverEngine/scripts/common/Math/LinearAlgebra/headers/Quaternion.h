#pragma once

namespace ForiverEngine
{
	struct Vector3;

	struct Quaternion
	{
		float x;
		float y;
		float z;
		float w;

		constexpr Quaternion() noexcept : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

		constexpr Quaternion(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
		constexpr Quaternion(const Quaternion& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}
		constexpr Quaternion(Quaternion&& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}

		static constexpr Quaternion Zero() noexcept { return Quaternion(0.0f, 0.0f, 0.0f, 0.0f); }
		static constexpr Quaternion Identity() noexcept { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

		static Quaternion FromAxisAngle(const Vector3& axis, float angleRad) noexcept;
		static Quaternion VectorToVector(const Vector3& from, const Vector3& to) noexcept;

		Quaternion& operator=(const Quaternion& other) noexcept;
		Quaternion& operator=(Quaternion&& other) noexcept;

		bool operator==(const Quaternion& other) const noexcept;
		bool operator!=(const Quaternion& other) const noexcept;

		Quaternion operator*(const Quaternion& other) const noexcept;
		Vector3 operator*(const Vector3& vec) const noexcept;

		Quaternion& operator*=(const Quaternion& other) noexcept;

		float LenSq() const noexcept;
		float Len() const noexcept;

		Quaternion Normalized() const noexcept;
		Quaternion& Norm() noexcept;

		Quaternion Conjugate() const noexcept;

		static Quaternion Lerp(const Quaternion& from, const Quaternion& to, float t) noexcept;
		static Quaternion Slerp(const Quaternion& from, const Quaternion& to, float t) noexcept;
	};
}
