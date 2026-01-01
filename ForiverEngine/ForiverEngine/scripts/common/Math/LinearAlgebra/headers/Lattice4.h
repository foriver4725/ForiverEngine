#pragma once

namespace ForiverEngine
{
	struct Lattice3;
	struct Vector4;

	struct Lattice4
	{
		int x;
		int y;
		int z;
		int w;

		constexpr Lattice4() noexcept : x(0), y(0), z(0), w(1) {}

		constexpr Lattice4(int x, int y, int z, int w = 1) noexcept : x(x), y(y), z(z), w(w) {}
		constexpr Lattice4(const Lattice4& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}
		constexpr Lattice4(Lattice4&& other) noexcept : x(other.x), y(other.y), z(other.z), w(other.w) {}

		constexpr Lattice4(float x, float y, float z, float w = 1.0f) noexcept : x(static_cast<int>(x)), y(static_cast<int>(y)), z(static_cast<int>(z)), w(static_cast<int>(w)) {}
		explicit Lattice4(const Vector4& vec) noexcept;
		explicit Lattice4(Vector4&& vec) noexcept;

		explicit Lattice4(const Lattice3& lattice, int w = 1) noexcept;
		explicit Lattice4(Lattice3&& lattice, int w = 1) noexcept;

		Lattice4& operator=(const Lattice4& other) noexcept;
		Lattice4& operator=(Lattice4&& other) noexcept;

		bool operator==(const Lattice4& other) const noexcept;
		bool operator!=(const Lattice4& other) const noexcept;

		Lattice4 operator+() const noexcept;
		Lattice4 operator-() const noexcept;

		Lattice4 operator+(const Lattice4& other) const noexcept;
		Lattice4 operator-(const Lattice4& other) const noexcept;
		Lattice4 operator*(int scalar) const noexcept;
		Lattice4 operator/(int scalar) const noexcept;
		friend Lattice4 operator*(int scalar, const Lattice4& lattice) noexcept;

		Lattice4& operator+=(const Lattice4& other) noexcept;
		Lattice4& operator-=(const Lattice4& other) noexcept;
		Lattice4& operator*=(int scalar) noexcept;
		Lattice4& operator/=(int scalar) noexcept;
	};
}