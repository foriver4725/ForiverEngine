#pragma once

#include <scripts/common/IncludeInternal.h>
#include <scripts/common/Math/Defines.h>

#include "./Vector2.h"

namespace ForiverEngine
{
	struct Lattice2
	{
		int x;
		int y;

		constexpr Lattice2() noexcept : x(0), y(0) {}
		constexpr Lattice2(int x, int y) noexcept : x(x), y(y) {}
		constexpr Lattice2(const Lattice2& other) noexcept : x(other.x), y(other.y) {}
		constexpr Lattice2(Lattice2&& other) noexcept : x(other.x), y(other.y) {}

		constexpr Lattice2(const Vector2& vec2) noexcept
			: x(static_cast<int>(vec2.x)), y(static_cast<int>(vec2.y))
		{
		}

		static constexpr Lattice2 Zero() noexcept { return Lattice2(0, 0); }
		static constexpr Lattice2 One() noexcept { return Lattice2(1, 1); }
		static constexpr Lattice2 Right() noexcept { return Lattice2(1, 0); }
		static constexpr Lattice2 Left() noexcept { return Lattice2(-1, 0); }
		static constexpr Lattice2 Up() noexcept { return Lattice2(0, 1); }
		static constexpr Lattice2 Down() noexcept { return Lattice2(0, -1); }

		Lattice2& operator=(const Lattice2& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
			}
			return *this;
		}
		Lattice2& operator=(Lattice2&& other) noexcept
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
			}
			return *this;
		}

		bool operator==(const Lattice2& other) const noexcept
		{
			return x == other.x && y == other.y;
		}
		bool operator!=(const Lattice2& other) const noexcept
		{
			return !(*this == other);
		}

		Lattice2 operator+() const noexcept
		{
			return *this;
		}
		Lattice2 operator-() const noexcept
		{
			return Lattice2(-x, -y);
		}

		Lattice2 operator+(const Lattice2& other) const noexcept
		{
			return Lattice2(x + other.x, y + other.y);
		}
		Lattice2 operator-(const Lattice2& other) const noexcept
		{
			return Lattice2(x - other.x, y - other.y);
		}
		Lattice2 operator*(int scalar) const noexcept
		{
			return Lattice2(x * scalar, y * scalar);
		}
		Lattice2 operator/(int scalar) const noexcept
		{
			if (scalar == 0)
			{
				return Lattice2(0, 0);
			}
			return Lattice2(x / scalar, y / scalar);
		}
		friend Lattice2 operator*(int scalar, const Lattice2& lattice) noexcept
		{
			return Lattice2(lattice.x * scalar, lattice.y * scalar);
		}

		Lattice2& operator+=(const Lattice2& other) noexcept
		{
			x += other.x;
			y += other.y;

			return *this;
		}
		Lattice2& operator-=(const Lattice2& other) noexcept
		{
			x -= other.x;
			y -= other.y;

			return *this;
		}
		Lattice2& operator*=(int scalar) noexcept
		{
			x *= scalar;
			y *= scalar;

			return *this;
		}
		Lattice2& operator/=(int scalar) noexcept
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
	};
}
