#include "../headers/Vector4.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Lattice4.h"
#include "../headers/Vector3.h"

namespace ForiverEngine
{
	Vector4::Vector4(const Vector3& vec, float w) noexcept : x(vec.x), y(vec.y), z(vec.z), w(w) {}
	Vector4::Vector4(const Lattice4& lattice) noexcept : x(static_cast<float>(lattice.x)), y(static_cast<float>(lattice.y)), z(static_cast<float>(lattice.z)), w(static_cast<float>(lattice.w)) {}

	Vector4& Vector4::operator=(const Vector4& other) noexcept
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
	Vector4& Vector4::operator=(Vector4&& other) noexcept
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

	bool Vector4::operator==(const Vector4& other) const noexcept
	{
		return std::abs(x - other.x) < Epsilon
			&& std::abs(y - other.y) < Epsilon
			&& std::abs(z - other.z) < Epsilon
			&& std::abs(w - other.w) < Epsilon;
	}
	bool Vector4::operator!=(const Vector4& other) const noexcept
	{
		return !(*this == other);
	}

	Vector4 Vector4::operator+() const noexcept
	{
		return *this;
	}
	Vector4 Vector4::operator-() const noexcept
	{
		return Vector4(-x, -y, -z, -w);
	}

	Vector4 Vector4::operator+(const Vector4& other) const noexcept
	{
		return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
	}
	Vector4 Vector4::operator-(const Vector4& other) const noexcept
	{
		return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
	}
	Vector4 Vector4::operator*(float scalar) const noexcept
	{
		return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
	}
	Vector4 Vector4::operator/(float scalar) const noexcept
	{
		if (std::abs(scalar) < Epsilon)
		{
			return Vector4(0, 0, 0, 0);
		}
		return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
	}
	Vector4 operator*(float scalar, const Vector4& vec) noexcept
	{
		return Vector4(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
	}

	Vector4& Vector4::operator+=(const Vector4& other) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;

		return *this;
	}
	Vector4& Vector4::operator-=(const Vector4& other) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;

		return *this;
	}
	Vector4& Vector4::operator*=(float scalar) noexcept
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;

		return *this;
	}
	Vector4& Vector4::operator/=(float scalar) noexcept
	{
		if (std::abs(scalar) < Epsilon)
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
			w = 0.0f;
		}
		else
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
		}

		return *this;
	}

	float Vector4::LenSq() const noexcept
	{
		return x * x + y * y + z * z + w * w;
	}
	float Vector4::Len() const noexcept
	{
		return std::sqrt(LenSq());
	}

	Vector4 Vector4::Normed() const noexcept
	{
		const float len = Len();
		if (std::abs(len) < Epsilon)
		{
			return Vector4(0, 0, 0, 0);
		}
		return *this / len;
	}
	Vector4& Vector4::Norm() noexcept
	{
		*this = Normed();
		return *this;
	}

	float Vector4::Dot(const Vector4& a, const Vector4& b) noexcept
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	Vector4 Vector4::Lerp(const Vector4& a, const Vector4& b, float t) noexcept
	{
		float _t = std::clamp(t, 0.0f, 1.0f);
		return a + (b - a) * _t;
	}
	Vector4 Vector4::Slerp(const Vector4& a, const Vector4& b, float t) noexcept
	{
		float _t = std::clamp(t, 0.0f, 1.0f);
		float dot = Dot(a.Normed(), b.Normed());
		dot = std::clamp(dot, -1.0f, 1.0f);
		float theta = std::acos(dot) * _t;
		Vector4 relativeVec = (b - a * dot).Normed();
		return (a * std::cos(theta)) + (relativeVec * std::sin(theta));
	}
}
