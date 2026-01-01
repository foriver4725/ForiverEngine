#include "../headers/Lattice3.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Lattice2.h"
#include "../headers/Vector3.h"

namespace ForiverEngine
{
	Lattice3::Lattice3(const Vector3& vec) noexcept : x(static_cast<int>(vec.x)), y(static_cast<int>(vec.y)), z(static_cast<int>(vec.z)) {}
	Lattice3::Lattice3(Vector3&& vec) noexcept : x(static_cast<int>(vec.x)), y(static_cast<int>(vec.y)), z(static_cast<int>(vec.z)) {}

	Lattice3::Lattice3(const Lattice2& lattice, int z) noexcept : x(lattice.x), y(lattice.y), z(z) {}
	Lattice3::Lattice3(Lattice2&& lattice, int z) noexcept : x(lattice.x), y(lattice.y), z(z) {}

	Lattice3& Lattice3::operator=(const Lattice3& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}
		return *this;
	}
	Lattice3& Lattice3::operator=(Lattice3&& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}
		return *this;
	}

	bool Lattice3::operator==(const Lattice3& other) const noexcept
	{
		return x == other.x && y == other.y && z == other.z;
	}
	bool Lattice3::operator!=(const Lattice3& other) const noexcept
	{
		return !(*this == other);
	}

	Lattice3 Lattice3::operator+() const noexcept
	{
		return *this;
	}
	Lattice3 Lattice3::operator-() const noexcept
	{
		return Lattice3(-x, -y, -z);
	}

	Lattice3 Lattice3::operator+(const Lattice3& other) const noexcept
	{
		return Lattice3(x + other.x, y + other.y, z + other.z);
	}
	Lattice3 Lattice3::operator-(const Lattice3& other) const noexcept
	{
		return Lattice3(x - other.x, y - other.y, z - other.z);
	}
	Lattice3 Lattice3::operator*(int scalar) const noexcept
	{
		return Lattice3(x * scalar, y * scalar, z * scalar);
	}
	Lattice3 Lattice3::operator/(int scalar) const noexcept
	{
		if (scalar == 0)
		{
			return Lattice3(0, 0, 0);
		}
		return Lattice3(x / scalar, y / scalar, z / scalar);
	}
	Lattice3 operator*(int scalar, const Lattice3& lattice) noexcept
	{
		return Lattice3(lattice.x * scalar, lattice.y * scalar, lattice.z * scalar);
	}

	Lattice3& Lattice3::operator+=(const Lattice3& other) noexcept
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}
	Lattice3& Lattice3::operator-=(const Lattice3& other) noexcept
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}
	Lattice3& Lattice3::operator*=(int scalar) noexcept
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;

		return *this;
	}
	Lattice3& Lattice3::operator/=(int scalar) noexcept
	{
		if (scalar == 0)
		{
			x = 0;
			y = 0;
			z = 0;
		}
		else
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
		}

		return *this;
	}
}
