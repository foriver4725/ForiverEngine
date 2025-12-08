#pragma once

#include <DirectXMath.h>

#include <cmath>
#include <algorithm>

namespace ForiverEngine
{
	constexpr float VectorEpsilon = 1e-6f;

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

		constexpr DirectX::XMFLOAT2 ToDirectXVector() const noexcept { return DirectX::XMFLOAT2(x, y); }

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
			return std::abs(x - other.x) < VectorEpsilon
				&& std::abs(y - other.y) < VectorEpsilon;
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
			if (std::abs(scalar) < VectorEpsilon)
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
			if (std::abs(scalar) < VectorEpsilon)
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
			if (std::abs(len) < VectorEpsilon)
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

	struct Vector3
	{
		float x;
		float y;
		float z;

		constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
		constexpr Vector3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
		constexpr Vector3(const Vector3& other) noexcept : x(other.x), y(other.y), z(other.z) {}
		constexpr Vector3(Vector3&& other) noexcept : x(other.x), y(other.y), z(other.z) {}

		constexpr Vector3(const Vector2& vec2) noexcept : x(vec2.x), y(vec2.y), z(0.0f) {}
		constexpr Vector3(const Vector2& vec2, float z) noexcept : x(vec2.x), y(vec2.y), z(z) {}

		constexpr DirectX::XMFLOAT3 ToDirectXVector() const noexcept { return DirectX::XMFLOAT3(x, y, z); }

		static constexpr Vector3 Zero() noexcept { return Vector3(0.0f, 0.0f, 0.0f); }
		static constexpr Vector3 One() noexcept { return Vector3(1.0f, 1.0f, 1.0f); }
		static constexpr Vector3 Right() noexcept { return Vector3(1.0f, 0.0f, 0.0f); }
		static constexpr Vector3 Left() noexcept { return Vector3(-1.0f, 0.0f, 0.0f); }
		static constexpr Vector3 Up() noexcept { return Vector3(0.0f, 1.0f, 0.0f); }
		static constexpr Vector3 Down() noexcept { return Vector3(0.0f, -1.0f, 0.0f); }
		static constexpr Vector3 Forward() noexcept { return Vector3(0.0f, 0.0f, 1.0f); }
		static constexpr Vector3 Backward() noexcept { return Vector3(0.0f, 0.0f, -1.0f); }

		Vector3& operator=(const Vector3& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
				z = other.z;
			}
			return *this;
		}
		Vector3& operator=(Vector3&& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
				z = other.z;
			}
			return *this;
		}

		bool operator==(const Vector3& other) const noexcept
		{
			return std::abs(x - other.x) < VectorEpsilon
				&& std::abs(y - other.y) < VectorEpsilon
				&& std::abs(z - other.z) < VectorEpsilon;
		}
		bool operator!=(const Vector3& other) const noexcept
		{
			return !(*this == other);
		}

		Vector3 operator+(const Vector3& other) const noexcept
		{
			return Vector3(x + other.x, y + other.y, z + other.z);
		}
		Vector3 operator-(const Vector3& other) const noexcept
		{
			return Vector3(x - other.x, y - other.y, z - other.z);
		}
		Vector3 operator*(float scalar) const noexcept
		{
			return Vector3(x * scalar, y * scalar, z * scalar);
		}
		Vector3 operator/(float scalar) const noexcept
		{
			if (std::abs(scalar) < VectorEpsilon)
			{
				return Vector3(0.0f, 0.0f, 0.0f);
			}
			return Vector3(x / scalar, y / scalar, z / scalar);
		}
		friend Vector3 operator*(float scalar, const Vector3& vec) noexcept
		{
			return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
		}

		void operator+=(const Vector3& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;
		}
		void operator-=(const Vector3& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
		}
		void operator*=(float scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}
		void operator/=(float scalar) noexcept
		{
			if (std::abs(scalar) < VectorEpsilon)
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
		}

		float LenSq() const noexcept
		{
			return x * x + y * y + z * z;
		}
		float Len() const noexcept
		{
			return std::sqrt(LenSq());
		}

		Vector3 Normed() const noexcept
		{
			const float len = Len();
			if (std::abs(len) < VectorEpsilon)
			{
				return Vector3(0.0f, 0.0f, 0.0f);
			}
			return *this / len;
		}
		void Norm() noexcept
		{
			*this = Normed();
		}

		static float Dot(const Vector3& lhs, const Vector3& rhs) noexcept
		{
			return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}
		static Vector3 Cross(const Vector3& lhs, const Vector3& rhs) noexcept
		{
			return Vector3(
				lhs.y * rhs.z - lhs.z * rhs.y,
				lhs.z * rhs.x - lhs.x * rhs.z,
				lhs.x * rhs.y - lhs.y * rhs.x
			);
		}

		static Vector3 Lerp(const Vector3& from, const Vector3& to, float t) noexcept
		{
			const float _t = std::clamp(t, 0.0f, 1.0f);
			return from + (to - from) * _t;
		}
		static Vector3 Reflect(const Vector3& vec, const Vector3& normal) noexcept
		{
			const Vector3 _normal = normal.Normed();
			const float dot = Dot(vec, _normal);
			return vec - 2.0f * dot * _normal;
		}
	};

	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;

		constexpr Vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
		constexpr Vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
		constexpr Vector4(const Vector4& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}
		constexpr Vector4(Vector4&& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}

		constexpr Vector4(const Vector3& v) noexcept : x(v.x), y(v.y), z(v.z), w(1.0f) {}
		constexpr Vector4(const Vector3& v, float w) noexcept : x(v.x), y(v.y), z(v.z), w(w) {}

		constexpr DirectX::XMFLOAT4 ToDirectXVector() const noexcept { return DirectX::XMFLOAT4(x, y, z, w); }

		Vector4& operator=(const Vector4& other) noexcept
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
		Vector4& operator=(Vector4&& other) noexcept
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

		bool operator==(const Vector4& other) const noexcept
		{
			return std::abs(x - other.x) < VectorEpsilon
				&& std::abs(y - other.y) < VectorEpsilon
				&& std::abs(z - other.z) < VectorEpsilon
				&& std::abs(w - other.w) < VectorEpsilon;
		}
		bool operator!=(const Vector4& other) const noexcept
		{
			return !(*this == other);
		}

		Vector4 operator+(const Vector4& other) const noexcept
		{
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
		}
		Vector4 operator-(const Vector4& other) const noexcept
		{
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
		}
		Vector4 operator*(float scalar) const noexcept
		{
			return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
		}
		Vector4 operator/(float scalar) const noexcept
		{
			if (std::abs(scalar) < VectorEpsilon)
			{
				return Vector4(0, 0, 0, 0);
			}
			return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
		}
		friend Vector4 operator*(float scalar, const Vector4& vec) noexcept
		{
			return Vector4(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
		}

		float LenSq() const noexcept
		{
			return x * x + y * y + z * z + w * w;
		}
		float Len() const noexcept
		{
			return std::sqrt(LenSq());
		}

		Vector4 Normed() const noexcept
		{
			const float len = Len();
			if (std::abs(len) < VectorEpsilon)
			{
				return Vector4(0, 0, 0, 0);
			}
			return *this / len;
		}
		void Norm() noexcept
		{
			*this = Normed();
		}

		static float Dot(const Vector4& a, const Vector4& b) noexcept
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		static Vector4 Lerp(const Vector4& a, const Vector4& b, float t) noexcept
		{
			float _t = std::clamp(t, 0.0f, 1.0f);
			return a + (b - a) * _t;
		}
	};
}
