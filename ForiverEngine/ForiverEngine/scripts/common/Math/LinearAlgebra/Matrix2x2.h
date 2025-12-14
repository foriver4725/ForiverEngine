#pragma once

#include <scripts/common/Math/Defines.h>
#include "./Vector2.h"

namespace ForiverEngine
{
	// 列優先
	struct Matrix2x2
	{
		float c0r0; float c0r1;
		float c1r0; float c1r1;

		constexpr Matrix2x2() noexcept
			: c0r0(1.0f), c1r0(0.0f)
			, c0r1(0.0f), c1r1(1.0f)
		{
		}
		constexpr Matrix2x2(
			float c0r0, float c1r0,
			float c0r1, float c1r1
		) noexcept
			: c0r0(c0r0), c1r0(c1r0)
			, c0r1(c0r1), c1r1(c1r1)
		{
		}
		constexpr Matrix2x2(const Matrix2x2& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0)
			, c0r1(other.c0r1), c1r1(other.c1r1)
		{
		}
		constexpr Matrix2x2(Matrix2x2&& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0)
			, c0r1(other.c0r1), c1r1(other.c1r1)
		{
		}

		static constexpr Matrix2x2 Identity() noexcept
		{
			return Matrix2x2(
				1.0f, 0.0f,
				0.0f, 1.0f
			);
		}
		static constexpr Matrix2x2 Zero() noexcept
		{
			return Matrix2x2(
				0.0f, 0.0f,
				0.0f, 0.0f
			);
		}

		static constexpr Matrix2x2 FromColumnVectors(
			const Vector2& c0,
			const Vector2& c1
		) noexcept
		{
			return Matrix2x2(
				c0.x, c1.x,
				c0.y, c1.y
			);
		}
		static constexpr Matrix2x2 FromRowVectors(
			const Vector2& r0,
			const Vector2& r1
		) noexcept
		{
			return Matrix2x2(
				r0.x, r0.y,
				r1.x, r1.y
			);
		}

		static Matrix2x2 Scale(float sx, float sy) noexcept
		{
			return Matrix2x2(
				sx, 0.0f,
				0.0f, sy
			);
		}
		static Matrix2x2 Scale(const Vector2& scale) noexcept
		{
			return Scale(scale.x, scale.y);
		}
		static Matrix2x2 Rotate(float angleRad) noexcept
		{
			const float cos = std::cos(angleRad);
			const float sin = std::sin(angleRad);
			return Matrix2x2(
				cos, -sin,
				sin, cos
			);
		}

		Matrix2x2& operator=(const Matrix2x2& other) noexcept
		{
			if (this != &other)
			{
				c0r0 = other.c0r0; c1r0 = other.c1r0;
				c0r1 = other.c0r1; c1r1 = other.c1r1;
			}
			return *this;
		}
		Matrix2x2& operator=(Matrix2x2&& other) noexcept
		{
			if (this != &other)
			{
				c0r0 = other.c0r0; c1r0 = other.c1r0;
				c0r1 = other.c0r1; c1r1 = other.c1r1;
			}
			return *this;
		}

		bool operator==(const Matrix2x2& other) const noexcept
		{
			return std::abs(c0r0 - other.c0r0) < Epsilon &&
				std::abs(c1r0 - other.c1r0) < Epsilon &&
				std::abs(c0r1 - other.c0r1) < Epsilon &&
				std::abs(c1r1 - other.c1r1) < Epsilon;
		}
		bool operator!=(const Matrix2x2& other) const noexcept
		{
			return !(*this == other);
		}

		Matrix2x2 operator+() const noexcept
		{
			return *this;
		}
		Matrix2x2 operator-() const noexcept
		{
			return Matrix2x2(
				-c0r0, -c1r0,
				-c0r1, -c1r1
			);
		}

		Matrix2x2 operator+(const Matrix2x2& other) const noexcept
		{
			return Matrix2x2(
				c0r0 + other.c0r0, c1r0 + other.c1r0,
				c0r1 + other.c0r1, c1r1 + other.c1r1
			);
		}
		Matrix2x2 operator-(const Matrix2x2& other) const noexcept
		{
			return Matrix2x2(
				c0r0 - other.c0r0, c1r0 - other.c1r0,
				c0r1 - other.c0r1, c1r1 - other.c1r1
			);
		}
		Matrix2x2 operator*(float scalar) const noexcept
		{
			return Matrix2x2(
				c0r0 * scalar, c1r0 * scalar,
				c0r1 * scalar, c1r1 * scalar
			);
		}
		Matrix2x2 operator*(const Matrix2x2& other) const noexcept
		{
			return Matrix2x2(
				c0r0 * other.c0r0 + c1r0 * other.c0r1,
				c0r0 * other.c1r0 + c1r0 * other.c1r1,
				c0r1 * other.c0r0 + c1r1 * other.c0r1,
				c0r1 * other.c1r0 + c1r1 * other.c1r1
			);
		}
		Vector2 operator*(const Vector2& vec) const noexcept
		{
			return Vector2(
				c0r0 * vec.x + c1r0 * vec.y,
				c0r1 * vec.x + c1r1 * vec.y
			);
		}
		Matrix2x2 operator/(float scalar) const noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				return Matrix2x2::Zero();
			}
			return Matrix2x2(
				c0r0 / scalar, c1r0 / scalar,
				c0r1 / scalar, c1r1 / scalar
			);
		}
		friend Matrix2x2 operator*(float scalar, const Matrix2x2& mat) noexcept
		{
			return Matrix2x2(
				mat.c0r0 * scalar, mat.c1r0 * scalar,
				mat.c0r1 * scalar, mat.c1r1 * scalar
			);
		}

		Matrix2x2& operator+=(const Matrix2x2& other) noexcept
		{
			c0r0 += other.c0r0; c1r0 += other.c1r0;
			c0r1 += other.c0r1; c1r1 += other.c1r1;

			return *this;
		}
		Matrix2x2& operator-=(const Matrix2x2& other) noexcept
		{
			c0r0 -= other.c0r0; c1r0 -= other.c1r0;
			c0r1 -= other.c0r1; c1r1 -= other.c1r1;

			return *this;
		}
		Matrix2x2& operator*=(float scalar) noexcept
		{
			c0r0 *= scalar; c1r0 *= scalar;
			c0r1 *= scalar; c1r1 *= scalar;

			return *this;
		}
		Matrix2x2& operator*=(const Matrix2x2& other) noexcept
		{
			*this = Matrix2x2(
				c0r0 * other.c0r0 + c1r0 * other.c0r1,
				c0r0 * other.c1r0 + c1r0 * other.c1r1,
				c0r1 * other.c0r0 + c1r1 * other.c0r1,
				c0r1 * other.c1r0 + c1r1 * other.c1r1
			);

			return *this;
		}
		Matrix2x2& operator/=(float scalar) noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				*this = Matrix2x2::Zero();
			}
			else
			{
				c0r0 /= scalar; c1r0 /= scalar;
				c0r1 /= scalar; c1r1 /= scalar;
			}

			return *this;
		}

		float Determinant() const noexcept
		{
			return c0r0 * c1r1 - c1r0 * c0r1;
		}

		Matrix2x2 Transposed() const noexcept
		{
			return Matrix2x2(
				c0r0, c0r1,
				c1r0, c1r1
			);
		}
		Matrix2x2& Transpose() noexcept
		{
			*this = Matrix2x2(
				c0r0, c0r1,
				c1r0, c1r1
			);

			return *this;
		}
	};
}
