#include "../headers/Matrix2x2.h"

#include <scripts/common/Math/Defines.h>
#include <cmath>
#include <algorithm>
#include "../headers/Vector2.h"

namespace ForiverEngine
{
	Matrix2x2 Matrix2x2::FromColumnVectors(
		const Vector2& c0,
		const Vector2& c1
	) noexcept
	{
		return Matrix2x2(
			c0.x, c1.x,
			c0.y, c1.y
		);
	}
	Matrix2x2 Matrix2x2::FromRowVectors(
		const Vector2& r0,
		const Vector2& r1
	) noexcept
	{
		return Matrix2x2(
			r0.x, r0.y,
			r1.x, r1.y
		);
	}

	Matrix2x2 Matrix2x2::Scale(float sx, float sy) noexcept
	{
		return Matrix2x2(
			sx, 0.0f,
			0.0f, sy
		);
	}
	Matrix2x2 Matrix2x2::Scale(const Vector2& scale) noexcept
	{
		return Scale(scale.x, scale.y);
	}
	Matrix2x2 Matrix2x2::Rotate(float angleRad) noexcept
	{
		const float cos = std::cos(angleRad);
		const float sin = std::sin(angleRad);
		return Matrix2x2(
			cos, -sin,
			sin, cos
		);
	}

	Matrix2x2& Matrix2x2::operator=(const Matrix2x2& other) noexcept
	{
		if (this != &other)
		{
			c0r0 = other.c0r0; c1r0 = other.c1r0;
			c0r1 = other.c0r1; c1r1 = other.c1r1;
		}
		return *this;
	}
	Matrix2x2& Matrix2x2::operator=(Matrix2x2&& other) noexcept
	{
		if (this != &other)
		{
			c0r0 = other.c0r0; c1r0 = other.c1r0;
			c0r1 = other.c0r1; c1r1 = other.c1r1;
		}
		return *this;
	}

	bool Matrix2x2::operator==(const Matrix2x2& other) const noexcept
	{
		return std::abs(c0r0 - other.c0r0) < Epsilon &&
			std::abs(c1r0 - other.c1r0) < Epsilon &&
			std::abs(c0r1 - other.c0r1) < Epsilon &&
			std::abs(c1r1 - other.c1r1) < Epsilon;
	}
	bool Matrix2x2::operator!=(const Matrix2x2& other) const noexcept
	{
		return !(*this == other);
	}

	Matrix2x2 Matrix2x2::operator+() const noexcept
	{
		return *this;
	}
	Matrix2x2 Matrix2x2::operator-() const noexcept
	{
		return Matrix2x2(
			-c0r0, -c1r0,
			-c0r1, -c1r1
		);
	}

	Matrix2x2 Matrix2x2::operator+(const Matrix2x2& other) const noexcept
	{
		return Matrix2x2(
			c0r0 + other.c0r0, c1r0 + other.c1r0,
			c0r1 + other.c0r1, c1r1 + other.c1r1
		);
	}
	Matrix2x2 Matrix2x2::operator-(const Matrix2x2& other) const noexcept
	{
		return Matrix2x2(
			c0r0 - other.c0r0, c1r0 - other.c1r0,
			c0r1 - other.c0r1, c1r1 - other.c1r1
		);
	}
	Matrix2x2 Matrix2x2::operator*(float scalar) const noexcept
	{
		return Matrix2x2(
			c0r0 * scalar, c1r0 * scalar,
			c0r1 * scalar, c1r1 * scalar
		);
	}
	Matrix2x2 Matrix2x2::operator*(const Matrix2x2& other) const noexcept
	{
		return Matrix2x2(
			c0r0 * other.c0r0 + c1r0 * other.c0r1,
			c0r0 * other.c1r0 + c1r0 * other.c1r1,
			c0r1 * other.c0r0 + c1r1 * other.c0r1,
			c0r1 * other.c1r0 + c1r1 * other.c1r1
		);
	}
	Vector2 Matrix2x2::operator*(const Vector2& vec) const noexcept
	{
		return Vector2(
			c0r0 * vec.x + c1r0 * vec.y,
			c0r1 * vec.x + c1r1 * vec.y
		);
	}
	Matrix2x2 Matrix2x2::operator/(float scalar) const noexcept
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
	Matrix2x2 operator*(float scalar, const Matrix2x2& mat) noexcept
	{
		return Matrix2x2(
			mat.c0r0 * scalar, mat.c1r0 * scalar,
			mat.c0r1 * scalar, mat.c1r1 * scalar
		);
	}

	Matrix2x2& Matrix2x2::operator+=(const Matrix2x2& other) noexcept
	{
		c0r0 += other.c0r0; c1r0 += other.c1r0;
		c0r1 += other.c0r1; c1r1 += other.c1r1;

		return *this;
	}
	Matrix2x2& Matrix2x2::operator-=(const Matrix2x2& other) noexcept
	{
		c0r0 -= other.c0r0; c1r0 -= other.c1r0;
		c0r1 -= other.c0r1; c1r1 -= other.c1r1;

		return *this;
	}
	Matrix2x2& Matrix2x2::operator*=(float scalar) noexcept
	{
		c0r0 *= scalar; c1r0 *= scalar;
		c0r1 *= scalar; c1r1 *= scalar;

		return *this;
	}
	Matrix2x2& Matrix2x2::operator*=(const Matrix2x2& other) noexcept
	{
		*this = Matrix2x2(
			c0r0 * other.c0r0 + c1r0 * other.c0r1,
			c0r0 * other.c1r0 + c1r0 * other.c1r1,
			c0r1 * other.c0r0 + c1r1 * other.c0r1,
			c0r1 * other.c1r0 + c1r1 * other.c1r1
		);

		return *this;
	}
	Matrix2x2& Matrix2x2::operator/=(float scalar) noexcept
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

	float Matrix2x2::Determinant() const noexcept
	{
		return c0r0 * c1r1 - c1r0 * c0r1;
	}

	Matrix2x2 Matrix2x2::Transposed() const noexcept
	{
		return Matrix2x2(
			c0r0, c0r1,
			c1r0, c1r1
		);
	}
	Matrix2x2& Matrix2x2::Transpose() noexcept
	{
		*this = Matrix2x2(
			c0r0, c0r1,
			c1r0, c1r1
		);

		return *this;
	}
}
