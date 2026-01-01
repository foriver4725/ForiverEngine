#pragma once

namespace ForiverEngine
{
	struct Vector3;
	struct Vector4;
	struct Matrix3x3;
	struct Quaternion;

	// 列優先
	struct Matrix4x4
	{
		float c0r0; float c0r1; float c0r2; float c0r3;
		float c1r0; float c1r1; float c1r2; float c1r3;
		float c2r0; float c2r1; float c2r2; float c2r3;
		float c3r0; float c3r1; float c3r2; float c3r3;

		constexpr Matrix4x4() noexcept
			: c0r0(1.0f), c1r0(0.0f), c2r0(0.0f), c3r0(0.0f)
			, c0r1(0.0f), c1r1(1.0f), c2r1(0.0f), c3r1(0.0f)
			, c0r2(0.0f), c1r2(0.0f), c2r2(1.0f), c3r2(0.0f)
			, c0r3(0.0f), c1r3(0.0f), c2r3(0.0f), c3r3(1.0f)
		{
		}
		constexpr Matrix4x4(
			float c0r0, float c1r0, float c2r0, float c3r0,
			float c0r1, float c1r1, float c2r1, float c3r1,
			float c0r2, float c1r2, float c2r2, float c3r2,
			float c0r3, float c1r3, float c2r3, float c3r3
		) noexcept
			: c0r0(c0r0), c1r0(c1r0), c2r0(c2r0), c3r0(c3r0)
			, c0r1(c0r1), c1r1(c1r1), c2r1(c2r1), c3r1(c3r1)
			, c0r2(c0r2), c1r2(c1r2), c2r2(c2r2), c3r2(c3r2)
			, c0r3(c0r3), c1r3(c1r3), c2r3(c2r3), c3r3(c3r3)
		{
		}

		constexpr Matrix4x4(const Matrix4x4& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(other.c2r0), c3r0(other.c3r0)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(other.c2r1), c3r1(other.c3r1)
			, c0r2(other.c0r2), c1r2(other.c1r2), c2r2(other.c2r2), c3r2(other.c3r2)
			, c0r3(other.c0r3), c1r3(other.c1r3), c2r3(other.c2r3), c3r3(other.c3r3)
		{
		}
		constexpr Matrix4x4(Matrix4x4&& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(other.c2r0), c3r0(other.c3r0)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(other.c2r1), c3r1(other.c3r1)
			, c0r2(other.c0r2), c1r2(other.c1r2), c2r2(other.c2r2), c3r2(other.c3r2)
			, c0r3(other.c0r3), c1r3(other.c1r3), c2r3(other.c2r3), c3r3(other.c3r3)
		{
		}

		explicit Matrix4x4(const Matrix3x3& other) noexcept;
		explicit Matrix4x4(Matrix3x3&& other) noexcept;

		static constexpr Matrix4x4 Identity() noexcept
		{
			return Matrix4x4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}
		static constexpr Matrix4x4 Zero() noexcept
		{
			return Matrix4x4(
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f
			);
		}

		static Matrix4x4 FromColumnVectors(
			const Vector4& c0,
			const Vector4& c1,
			const Vector4& c2,
			const Vector4& c3
		) noexcept;
		static Matrix4x4 FromRowVectors(
			const Vector4& r0,
			const Vector4& r1,
			const Vector4& r2,
			const Vector4& r3
		) noexcept;

		static Matrix4x4 Scale(float sx, float sy, float sz) noexcept;
		static Matrix4x4 Scale(const Vector3& scale) noexcept;
		static Matrix4x4 Rotate(Quaternion quat) noexcept;
		static Matrix4x4 Translate(float tx, float ty, float tz) noexcept;
		static Matrix4x4 Translate(const Vector3& translation) noexcept;

		Matrix4x4& operator=(const Matrix4x4& other) noexcept;
		Matrix4x4& operator=(Matrix4x4&& other) noexcept;

		bool operator==(const Matrix4x4& other) const noexcept;
		bool operator!=(const Matrix4x4& other) const noexcept;

		Matrix4x4 operator+() const noexcept;
		Matrix4x4 operator-() const noexcept;

		Matrix4x4 operator+(const Matrix4x4& other) const noexcept;
		Matrix4x4 operator-(const Matrix4x4& other) const noexcept;
		Matrix4x4 operator*(float scalar) const noexcept;
		Matrix4x4 operator*(const Matrix4x4& other) const noexcept;
		Vector4 operator*(const Vector4& vec) const noexcept;
		Matrix4x4 operator/(float scalar) const noexcept;
		friend Matrix4x4 operator*(float scalar, const Matrix4x4& mat) noexcept;

		Matrix4x4& operator+=(const Matrix4x4& other) noexcept;
		Matrix4x4& operator-=(const Matrix4x4& other) noexcept;
		Matrix4x4& operator*=(float scalar) noexcept;
		Matrix4x4& operator*=(const Matrix4x4& other) noexcept;
		Matrix4x4& operator/=(float scalar) noexcept;

		float Determinant() const noexcept;
		Matrix4x4 Transposed() const noexcept;
		Matrix4x4& Transpose() noexcept;
	};
}
