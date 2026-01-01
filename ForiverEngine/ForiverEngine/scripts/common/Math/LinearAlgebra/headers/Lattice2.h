#pragma once

namespace ForiverEngine
{
	struct Vector2;

	struct Lattice2
	{
		int x;
		int y;

		constexpr Lattice2() noexcept : x(0), y(0) {}

		constexpr Lattice2(int x, int y) noexcept : x(x), y(y) {}
		constexpr Lattice2(const Lattice2& other) noexcept : x(other.x), y(other.y) {}
		constexpr Lattice2(Lattice2&& other) noexcept : x(other.x), y(other.y) {}

		constexpr Lattice2(float x, float y) noexcept : x(static_cast<int>(x)), y(static_cast<int>(y)) {}
		Lattice2(const Vector2& vec) noexcept;
		Lattice2(Vector2&& vec) noexcept;

		static constexpr Lattice2 Zero() noexcept { return Lattice2(0, 0); }
		static constexpr Lattice2 One() noexcept { return Lattice2(1, 1); }
		static constexpr Lattice2 Right() noexcept { return Lattice2(1, 0); }
		static constexpr Lattice2 Left() noexcept { return Lattice2(-1, 0); }
		static constexpr Lattice2 Up() noexcept { return Lattice2(0, 1); }
		static constexpr Lattice2 Down() noexcept { return Lattice2(0, -1); }

		Lattice2& operator=(const Lattice2& other) noexcept;
		Lattice2& operator=(Lattice2&& other) noexcept;

		bool operator==(const Lattice2& other) const noexcept;
		bool operator!=(const Lattice2& other) const noexcept;

		Lattice2 operator+() const noexcept;
		Lattice2 operator-() const noexcept;

		Lattice2 operator+(const Lattice2& other) const noexcept;
		Lattice2 operator-(const Lattice2& other) const noexcept;
		Lattice2 operator*(int scalar) const noexcept;
		Lattice2 operator/(int scalar) const noexcept;
		friend Lattice2 operator*(int scalar, const Lattice2& lattice) noexcept;

		Lattice2& operator+=(const Lattice2& other) noexcept;
		Lattice2& operator-=(const Lattice2& other) noexcept;
		Lattice2& operator*=(int scalar) noexcept;
		Lattice2& operator/=(int scalar) noexcept;
	};
}
