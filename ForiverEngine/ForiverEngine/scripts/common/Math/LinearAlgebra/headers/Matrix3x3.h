#pragma once

namespace ForiverEngine
{
	struct Vector3;
	struct Matrix2x2;
	struct Quaternion;

	// 列優先
	struct Matrix3x3
	{
		float c0r0; float c0r1; float c0r2;
		float c1r0; float c1r1; float c1r2;
		float c2r0; float c2r1; float c2r2;

		constexpr Matrix3x3() noexcept
			: c0r0(1.0f), c1r0(0.0f), c2r0(0.0f)
			, c0r1(0.0f), c1r1(1.0f), c2r1(0.0f)
			, c0r2(0.0f), c1r2(0.0f), c2r2(1.0f)
		{
		}
		constexpr Matrix3x3(
			float c0r0, float c1r0, float c2r0,
			float c0r1, float c1r1, float c2r1,
			float c0r2, float c1r2, float c2r2
		) noexcept
			: c0r0(c0r0), c1r0(c1r0), c2r0(c2r0)
			, c0r1(c0r1), c1r1(c1r1), c2r1(c2r1)
			, c0r2(c0r2), c1r2(c1r2), c2r2(c2r2)
		{
		}
		constexpr Matrix3x3(const Matrix3x3& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(other.c2r0)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(other.c2r1)
			, c0r2(other.c0r2), c1r2(other.c1r2), c2r2(other.c2r2)
		{
		}
		constexpr Matrix3x3(Matrix3x3&& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(other.c2r0)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(other.c2r1)
			, c0r2(other.c0r2), c1r2(other.c1r2), c2r2(other.c2r2)
		{
		}

		explicit Matrix3x3(const Matrix2x2& other) noexcept;
		explicit Matrix3x3(Matrix2x2&& other) noexcept;

		static constexpr Matrix3x3 Identity() noexcept
		{
			return Matrix3x3(
				1.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 1.0f
			);
		}
		static constexpr Matrix3x3 Zero() noexcept
		{
			return Matrix3x3(
				0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f
			);
		}

		static Matrix3x3 FromColumnVectors(
			const Vector3& c0,
			const Vector3& c1,
			const Vector3& c2
		) noexcept;
		static Matrix3x3 FromRowVectors(
			const Vector3& r0,
			const Vector3& r1,
			const Vector3& r2
		) noexcept;

		static Matrix3x3 Scale(float sx, float sy, float sz) noexcept;
		static Matrix3x3 Scale(const Vector3& scale) noexcept;
		static Matrix3x3 Rotate(Quaternion quat) noexcept;

		Matrix3x3& operator=(const Matrix3x3& other) noexcept;
		Matrix3x3& operator=(Matrix3x3&& other) noexcept;

		bool operator==(const Matrix3x3& other) const noexcept;
		bool operator!=(const Matrix3x3& other) const noexcept;

		Matrix3x3 operator+() const noexcept;
		Matrix3x3 operator-() const noexcept;

		Matrix3x3 operator+(const Matrix3x3& other) const noexcept;
		Matrix3x3 operator-(const Matrix3x3& other) const noexcept;
		Matrix3x3 operator*(float scalar) const noexcept;
		Matrix3x3 operator*(const Matrix3x3& other) const noexcept;
		Vector3 operator*(const Vector3& vec) const noexcept;
		Matrix3x3 operator/(float scalar) const noexcept;
		friend Matrix3x3 operator*(float scalar, const Matrix3x3& mat) noexcept;

		Matrix3x3& operator+=(const Matrix3x3& other) noexcept;
		Matrix3x3& operator-=(const Matrix3x3& other) noexcept;
		Matrix3x3& operator*=(float scalar) noexcept;
		Matrix3x3& operator*=(const Matrix3x3& other) noexcept;
		Matrix3x3& operator/=(float scalar) noexcept;

		float Determinant() const noexcept;

		Matrix3x3 Transposed() const noexcept;
		Matrix3x3& Transpose() noexcept;
	};
}
