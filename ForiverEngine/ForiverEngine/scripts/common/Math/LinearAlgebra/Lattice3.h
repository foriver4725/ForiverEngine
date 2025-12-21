#pragma once

#include <scripts/common/Math/Defines.h>
#include "./Vector3.h"

namespace ForiverEngine
{
	struct Lattice3
	{
		int x;
		int y;
		int z;

		constexpr Lattice3() noexcept : x(0), y(0), z(0) {}
		constexpr Lattice3(int x, int y, int z) noexcept : x(x), y(y), z(z) {}
		constexpr Lattice3(const Lattice3& other) noexcept : x(other.x), y(other.y), z(other.z) {}
		constexpr Lattice3(Lattice3&& other) noexcept : x(other.x), y(other.y), z(other.z) {}

		constexpr Lattice3(const Vector3& vec3) noexcept
			: x(static_cast<int>(vec3.x)), y(static_cast<int>(vec3.y)), z(static_cast<int>(vec3.z))
		{
		}

		Lattice3& operator=(const Lattice3& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
				z = other.z;
			}
			return *this;
		}
		Lattice3& operator=(Lattice3&& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
				z = other.z;
			}
			return *this;
		}

		bool operator==(const Lattice3& other) const noexcept
		{
			return x == other.x && y == other.y && z == other.z;
		}
		bool operator!=(const Lattice3& other) const noexcept
		{
			return !(*this == other);
		}

		Lattice3 operator+() const noexcept
		{
			return *this;
		}
		Lattice3 operator-() const noexcept
		{
			return Lattice3(-x, -y, -z);
		}

		Lattice3 operator+(const Lattice3& other) const noexcept
		{
			return Lattice3(x + other.x, y + other.y, z + other.z);
		}
		Lattice3 operator-(const Lattice3& other) const noexcept
		{
			return Lattice3(x - other.x, y - other.y, z - other.z);
		}
		Lattice3 operator*(int scalar) const noexcept
		{
			return Lattice3(x * scalar, y * scalar, z * scalar);
		}
		friend Lattice3 operator*(int scalar, const Lattice3& lattice) noexcept
		{
			return Lattice3(lattice.x * scalar, lattice.y * scalar, lattice.z * scalar);
		}

		Lattice3& operator+=(const Lattice3& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;

			return *this;
		}
		Lattice3& operator-=(const Lattice3& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;

			return *this;
		}
		Lattice3& operator*=(int scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;

			return *this;
		}
	};
}