#pragma once

namespace ForiverEngine
{
	struct Vector3;

	struct Lattice3
	{
		int x;
		int y;
		int z;

		constexpr Lattice3() noexcept : x(0), y(0), z(0) {}
		constexpr Lattice3(int x, int y, int z) noexcept : x(x), y(y), z(z) {}
		constexpr Lattice3(const Lattice3& other) noexcept : x(other.x), y(other.y), z(other.z) {}
		constexpr Lattice3(Lattice3&& other) noexcept : x(other.x), y(other.y), z(other.z) {}

		Lattice3(const Vector3& vec3) noexcept;

		static constexpr Lattice3 Zero() noexcept { return Lattice3(0, 0, 0); }
		static constexpr Lattice3 One() noexcept { return Lattice3(1, 1, 1); }
		static constexpr Lattice3 Right() noexcept { return Lattice3(1, 0, 0); }
		static constexpr Lattice3 Left() noexcept { return Lattice3(-1, 0, 0); }
		static constexpr Lattice3 Up() noexcept { return Lattice3(0, 1, 0); }
		static constexpr Lattice3 Down() noexcept { return Lattice3(0, -1, 0); }
		static constexpr Lattice3 Forward() noexcept { return Lattice3(0, 0, 1); }
		static constexpr Lattice3 Backward() noexcept { return Lattice3(0, 0, -1); }

		Lattice3& operator=(const Lattice3& other) noexcept;
		Lattice3& operator=(Lattice3&& other) noexcept;

		bool operator==(const Lattice3& other) const noexcept;
		bool operator!=(const Lattice3& other) const noexcept;

		Lattice3 operator+() const noexcept;
		Lattice3 operator-() const noexcept;

		Lattice3 operator+(const Lattice3& other) const noexcept;
		Lattice3 operator-(const Lattice3& other) const noexcept;
		Lattice3 operator*(int scalar) const noexcept;
		Lattice3 operator/(int scalar) const noexcept;
		friend Lattice3 operator*(int scalar, const Lattice3& lattice) noexcept;

		Lattice3& operator+=(const Lattice3& other) noexcept;
		Lattice3& operator-=(const Lattice3& other) noexcept;
		Lattice3& operator*=(int scalar) noexcept;
		Lattice3& operator/=(int scalar) noexcept;
	};
}