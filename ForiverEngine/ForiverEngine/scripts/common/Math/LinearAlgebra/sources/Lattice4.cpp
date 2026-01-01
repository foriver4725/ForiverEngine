#include "../headers/Lattice4.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Lattice3.h"
#include "../headers/Vector4.h"

namespace ForiverEngine
{
	Lattice4::Lattice4(const Lattice3& lattice, int w) noexcept : x(lattice.x), y(lattice.y), z(lattice.z), w(w) {}
	Lattice4::Lattice4(const Vector4& vec) noexcept : x(static_cast<int>(vec.x)), y(static_cast<int>(vec.y)), z(static_cast<int>(vec.z)), w(static_cast<int>(vec.w)) {}

	Lattice4& Lattice4::operator=(const Lattice4& other) noexcept
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
	Lattice4& Lattice4::operator=(Lattice4&& other) noexcept
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

	bool Lattice4::operator==(const Lattice4& other) const noexcept
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}
	bool Lattice4::operator!=(const Lattice4& other) const noexcept
	{
		return !(*this == other);
	}

	Lattice4 Lattice4::operator+() const noexcept
	{
		return *this;
	}
	Lattice4 Lattice4::operator-() const noexcept
	{
		return Lattice4(-x, -y, -z, -w);
	}

	Lattice4 Lattice4::operator+(const Lattice4& other) const noexcept
	{
		return Lattice4(x + other.x, y + other.y, z + other.z, w + other.w);
	}
	Lattice4 Lattice4::operator-(const Lattice4& other) const noexcept
	{
		return Lattice4(x - other.x, y - other.y, z - other.z, w - other.w);
	}
	Lattice4 Lattice4::operator*(int scalar) const noexcept
	{
		return Lattice4(x * scalar, y * scalar, z * scalar, w * scalar);
	}
	Lattice4 Lattice4::operator/(int scalar) const noexcept
	{
		if (scalar == 0)
		{
			return Lattice4(0, 0, 0, 0);
		}
		return Lattice4(x / scalar, y / scalar, z / scalar, w / scalar);
	}
	Lattice4 operator*(int scalar, const Lattice4& lattice) noexcept
	{
		return Lattice4(lattice.x * scalar, lattice.y * scalar, lattice.z * scalar, lattice.w * scalar);
	}

	Lattice4& Lattice4::operator+=(const Lattice4& other) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;

		return *this;
	}
	Lattice4& Lattice4::operator-=(const Lattice4& other) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;

		return *this;
	}
	Lattice4& Lattice4::operator*=(int scalar) noexcept
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;

		return *this;
	}
	Lattice4& Lattice4::operator/=(int scalar) noexcept
	{
		if (scalar == 0)
		{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
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
}
