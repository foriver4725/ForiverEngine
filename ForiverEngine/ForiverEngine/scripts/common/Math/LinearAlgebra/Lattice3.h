#pragma once

#include <scripts/common/IncludeInternal.h>
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

		static constexpr Lattice3 Zero() noexcept { return Lattice3(0, 0, 0); }
		static constexpr Lattice3 One() noexcept { return Lattice3(1, 1, 1); }
		static constexpr Lattice3 Right() noexcept { return Lattice3(1, 0, 0); }
		static constexpr Lattice3 Left() noexcept { return Lattice3(-1, 0, 0); }
		static constexpr Lattice3 Up() noexcept { return Lattice3(0, 1, 0); }
		static constexpr Lattice3 Down() noexcept { return Lattice3(0, -1, 0); }
		static constexpr Lattice3 Forward() noexcept { return Lattice3(0, 0, 1); }
		static constexpr Lattice3 Backward() noexcept { return Lattice3(0, 0, -1); }

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
		Lattice3 operator/(int scalar) const noexcept
		{
			if (scalar == 0)
			{
				return Lattice3(0, 0, 0);
			}
			return Lattice3(x / scalar, y / scalar, z / scalar);
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
		Lattice3& operator/=(int scalar) noexcept
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
	};
}