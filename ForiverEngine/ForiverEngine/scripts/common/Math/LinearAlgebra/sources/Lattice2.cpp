#include "../headers/Lattice2.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Vector2.h"

namespace ForiverEngine
{
	Lattice2::Lattice2(const Vector2& vec) noexcept : x(static_cast<int>(vec.x)), y(static_cast<int>(vec.y)) {}

	Lattice2& Lattice2::operator=(const Lattice2& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
		}
		return *this;
	}
	Lattice2& Lattice2::operator=(Lattice2&& other) noexcept
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
		}
		return *this;
	}

	bool Lattice2::operator==(const Lattice2& other) const noexcept
	{
		return x == other.x && y == other.y;
	}
	bool Lattice2::operator!=(const Lattice2& other) const noexcept
	{
		return !(*this == other);
	}

	Lattice2 Lattice2::operator+() const noexcept
	{
		return *this;
	}
	Lattice2 Lattice2::operator-() const noexcept
	{
		return Lattice2(-x, -y);
	}

	Lattice2 Lattice2::operator+(const Lattice2& other) const noexcept
	{
		return Lattice2(x + other.x, y + other.y);
	}
	Lattice2 Lattice2::operator-(const Lattice2& other) const noexcept
	{
		return Lattice2(x - other.x, y - other.y);
	}
	Lattice2 Lattice2::operator*(int scalar) const noexcept
	{
		return Lattice2(x * scalar, y * scalar);
	}
	Lattice2 Lattice2::operator/(int scalar) const noexcept
	{
		if (scalar == 0)
		{
			return Lattice2(0, 0);
		}
		return Lattice2(x / scalar, y / scalar);
	}
	Lattice2 operator*(int scalar, const Lattice2& lattice) noexcept
	{
		return Lattice2(lattice.x * scalar, lattice.y * scalar);
	}

	Lattice2& Lattice2::operator+=(const Lattice2& other) noexcept
	{
		x += other.x;
		y += other.y;

		return *this;
	}
	Lattice2& Lattice2::operator-=(const Lattice2& other) noexcept
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}
	Lattice2& Lattice2::operator*=(int scalar) noexcept
	{
		x *= scalar;
		y *= scalar;

		return *this;
	}
	Lattice2& Lattice2::operator/=(int scalar) noexcept
	{
		if (scalar == 0)
		{
			x = 0;
			y = 0;
		}
		else
		{
			x /= scalar;
			y /= scalar;
		}

		return *this;
	}
}
