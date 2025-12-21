#pragma once

#include <scripts/common/Math/Defines.h>
#include "./Vector4.h"

namespace ForiverEngine
{
	struct Lattice4
	{
		int x;
		int y;
		int z;
		int w;

		constexpr Lattice4() noexcept : x(0), y(0), z(0), w(0) {}
		constexpr Lattice4(int x, int y, int z, int w) noexcept : x(x), y(y), z(z), w(w) {}
		constexpr Lattice4(const Lattice4& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}
		constexpr Lattice4(Lattice4&& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}

		constexpr Lattice4(const Vector4& vec4) noexcept
			: x(static_cast<int>(vec4.x)), y(static_cast<int>(vec4.y)), z(static_cast<int>(vec4.z)), w(static_cast<int>(vec4.w))
		{
		}

		Lattice4& operator=(const Lattice4& other) noexcept
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
		Lattice4& operator=(Lattice4&& other) noexcept
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

		bool operator==(const Lattice4& other) const noexcept
		{
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}
		bool operator!=(const Lattice4& other) const noexcept
		{
			return !(*this == other);
		}

		Lattice4 operator+() const noexcept
		{
			return *this;
		}
		Lattice4 operator-() const noexcept
		{
			return Lattice4(-x, -y, -z, -w);
		}

		Lattice4 operator+(const Lattice4& other) const noexcept
		{
			return Lattice4(x + other.x, y + other.y, z + other.z, w + other.w);
		}
		Lattice4 operator-(const Lattice4& other) const noexcept
		{
			return Lattice4(x - other.x, y - other.y, z - other.z, w - other.w);
		}
		Lattice4 operator*(int scalar) const noexcept
		{
			return Lattice4(x * scalar, y * scalar, z * scalar, w * scalar);
		}
		friend Lattice4 operator*(int scalar, const Lattice4& lattice) noexcept
		{
			return Lattice4(lattice.x * scalar, lattice.y * scalar, lattice.z * scalar, lattice.w * scalar);
		}

		Lattice4& operator+=(const Lattice4& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;

			return *this;
		}
		Lattice4& operator-=(const Lattice4& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;

			return *this;
		}
		Lattice4& operator*=(int scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;

			return *this;
		}
	};
}