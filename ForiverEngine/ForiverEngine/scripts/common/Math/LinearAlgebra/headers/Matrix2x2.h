#pragma once

namespace ForiverEngine
{
	struct Vector2;

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

		static Matrix2x2 FromColumnVectors(
			const Vector2& c0,
			const Vector2& c1
		) noexcept;
		static Matrix2x2 FromRowVectors(
			const Vector2& r0,
			const Vector2& r1
		) noexcept;

		static Matrix2x2 Scale(float sx, float sy) noexcept;
		static Matrix2x2 Scale(const Vector2& scale) noexcept;
		static Matrix2x2 Rotate(float angleRad) noexcept;

		Matrix2x2& operator=(const Matrix2x2& other) noexcept;
		Matrix2x2& operator=(Matrix2x2&& other) noexcept;

		bool operator==(const Matrix2x2& other) const noexcept;
		bool operator!=(const Matrix2x2& other) const noexcept;

		Matrix2x2 operator+() const noexcept;
		Matrix2x2 operator-() const noexcept;

		Matrix2x2 operator+(const Matrix2x2& other) const noexcept;
		Matrix2x2 operator-(const Matrix2x2& other) const noexcept;
		Matrix2x2 operator*(float scalar) const noexcept;
		Matrix2x2 operator*(const Matrix2x2& other) const noexcept;
		Vector2 operator*(const Vector2& vec) const noexcept;
		Matrix2x2 operator/(float scalar) const noexcept;
		friend Matrix2x2 operator*(float scalar, const Matrix2x2& mat) noexcept;

		Matrix2x2& operator+=(const Matrix2x2& other) noexcept;
		Matrix2x2& operator-=(const Matrix2x2& other) noexcept;
		Matrix2x2& operator*=(float scalar) noexcept;
		Matrix2x2& operator*=(const Matrix2x2& other) noexcept;
		Matrix2x2& operator/=(float scalar) noexcept;

		float Determinant() const noexcept;

		Matrix2x2 Transposed() const noexcept;
		Matrix2x2& Transpose() noexcept;
	};
}
