#pragma once

#include "./Defines.h"
#include "./Vector3.h"
#include "./Quaternion.h"
#include "./Matrix2x2.h"

namespace ForiverEngine
{
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
		constexpr Matrix3x3(const Matrix2x2& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(0.0f)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(0.0f)
			, c0r2(0.0f), c1r2(0.0f), c2r2(1.0f)
		{
		}
		constexpr Matrix3x3(Matrix2x2&& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(0.0f)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(0.0f)
			, c0r2(0.0f), c1r2(0.0f), c2r2(1.0f)
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

		static constexpr Matrix3x3 FromColumnVectors(
			const Vector3& c0,
			const Vector3& c1,
			const Vector3& c2
		) noexcept
		{
			return Matrix3x3(
				c0.x, c1.x, c2.x,
				c0.y, c1.y, c2.y,
				c0.z, c1.z, c2.z
			);
		}
		static constexpr Matrix3x3 FromRowVectors(
			const Vector3& r0,
			const Vector3& r1,
			const Vector3& r2
		) noexcept
		{
			return Matrix3x3(
				r0.x, r0.y, r0.z,
				r1.x, r1.y, r1.z,
				r2.x, r2.y, r2.z
			);
		}

		static Matrix3x3 Scale(float sx, float sy, float sz) noexcept
		{
			return Matrix3x3(
				sx, 0.0f, 0.0f,
				0.0f, sy, 0.0f,
				0.0f, 0.0f, sz
			);
		}
		static Matrix3x3 Scale(const Vector3& scale) noexcept
		{
			return Scale(scale.x, scale.y, scale.z);
		}
		static Matrix3x3 Rotate(Quaternion quat) noexcept
		{
			const Quaternion q = quat.Normalized();

			const float xx = q.x * q.x;
			const float yy = q.y * q.y;
			const float zz = q.z * q.z;
			const float xy = q.x * q.y;
			const float xz = q.x * q.z;
			const float yz = q.y * q.z;
			const float wx = q.w * q.x;
			const float wy = q.w * q.y;
			const float wz = q.w * q.z;

			return Matrix3x3(
				// column 0
				1.0f - 2.0f * (yy + zz),
				2.0f * (xy + wz),
				2.0f * (xz - wy),

				// column 1
				2.0f * (xy - wz),
				1.0f - 2.0f * (xx + zz),
				2.0f * (yz + wx),

				// column 2
				2.0f * (xz + wy),
				2.0f * (yz - wx),
				1.0f - 2.0f * (xx + yy)
			);
		}

		Matrix3x3& operator=(const Matrix3x3& other) noexcept
		{
			if (this != &other)
			{
				c0r0 = other.c0r0; c1r0 = other.c1r0; c2r0 = other.c2r0;
				c0r1 = other.c0r1; c1r1 = other.c1r1; c2r1 = other.c2r1;
				c0r2 = other.c0r2; c1r2 = other.c1r2; c2r2 = other.c2r2;
			}
			return *this;
		}
		Matrix3x3& operator=(Matrix3x3&& other) noexcept
		{
			if (this != &other)
			{
				c0r0 = other.c0r0; c1r0 = other.c1r0; c2r0 = other.c2r0;
				c0r1 = other.c0r1; c1r1 = other.c1r1; c2r1 = other.c2r1;
				c0r2 = other.c0r2; c1r2 = other.c1r2; c2r2 = other.c2r2;
			}
			return *this;
		}

		bool operator==(const Matrix3x3& other) const noexcept
		{
			return std::abs(c0r0 - other.c0r0) < Epsilon && std::abs(c1r0 - other.c1r0) < Epsilon && std::abs(c2r0 - other.c2r0) < Epsilon &&
				std::abs(c0r1 - other.c0r1) < Epsilon && std::abs(c1r1 - other.c1r1) < Epsilon && std::abs(c2r1 - other.c2r1) < Epsilon &&
				std::abs(c0r2 - other.c0r2) < Epsilon && std::abs(c1r2 - other.c1r2) < Epsilon && std::abs(c2r2 - other.c2r2) < Epsilon;
		}
		bool operator!=(const Matrix3x3& other) const noexcept
		{
			return !(*this == other);
		}

		Matrix3x3 operator+() const noexcept
		{
			return *this;
		}
		Matrix3x3 operator-() const noexcept
		{
			return Matrix3x3(
				-c0r0, -c1r0, -c2r0,
				-c0r1, -c1r1, -c2r1,
				-c0r2, -c1r2, -c2r2
			);
		}

		Matrix3x3 operator+(const Matrix3x3& other) const noexcept
		{
			return Matrix3x3(
				c0r0 + other.c0r0, c1r0 + other.c1r0, c2r0 + other.c2r0,
				c0r1 + other.c0r1, c1r1 + other.c1r1, c2r1 + other.c2r1,
				c0r2 + other.c0r2, c1r2 + other.c1r2, c2r2 + other.c2r2
			);
		}
		Matrix3x3 operator-(const Matrix3x3& other) const noexcept
		{
			return Matrix3x3(
				c0r0 - other.c0r0, c1r0 - other.c1r0, c2r0 - other.c2r0,
				c0r1 - other.c0r1, c1r1 - other.c1r1, c2r1 - other.c2r1,
				c0r2 - other.c0r2, c1r2 - other.c1r2, c2r2 - other.c2r2
			);
		}
		Matrix3x3 operator*(float scalar) const noexcept
		{
			return Matrix3x3(
				c0r0 * scalar, c1r0 * scalar, c2r0 * scalar,
				c0r1 * scalar, c1r1 * scalar, c2r1 * scalar,
				c0r2 * scalar, c1r2 * scalar, c2r2 * scalar
			);
		}
		Matrix3x3 operator*(const Matrix3x3& other) const noexcept
		{
			return Matrix3x3(
				c0r0 * other.c0r0 + c1r0 * other.c0r1 + c2r0 * other.c0r2,
				c0r0 * other.c1r0 + c1r0 * other.c1r1 + c2r0 * other.c1r2,
				c0r0 * other.c2r0 + c1r0 * other.c2r1 + c2r0 * other.c2r2,
				c0r1 * other.c0r0 + c1r1 * other.c0r1 + c2r1 * other.c0r2,
				c0r1 * other.c1r0 + c1r1 * other.c1r1 + c2r1 * other.c1r2,
				c0r1 * other.c2r0 + c1r1 * other.c2r1 + c2r1 * other.c2r2,
				c0r2 * other.c0r0 + c1r2 * other.c0r1 + c2r2 * other.c0r2,
				c0r2 * other.c1r0 + c1r2 * other.c1r1 + c2r2 * other.c1r2,
				c0r2 * other.c2r0 + c1r2 * other.c2r1 + c2r2 * other.c2r2
			);
		}
		Vector3 operator*(const Vector3& vec) const noexcept
		{
			return Vector3(
				c0r0 * vec.x + c1r0 * vec.y + c2r0 * vec.z,
				c0r1 * vec.x + c1r1 * vec.y + c2r1 * vec.z,
				c0r2 * vec.x + c1r2 * vec.y + c2r2 * vec.z
			);
		}
		Matrix3x3 operator/(float scalar) const noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				return Matrix3x3::Zero();
			}
			return Matrix3x3(
				c0r0 / scalar, c1r0 / scalar, c2r0 / scalar,
				c0r1 / scalar, c1r1 / scalar, c2r1 / scalar,
				c0r2 / scalar, c1r2 / scalar, c2r2 / scalar
			);
		}
		friend Matrix3x3 operator*(float scalar, const Matrix3x3& mat) noexcept
		{
			return Matrix3x3(
				mat.c0r0 * scalar, mat.c1r0 * scalar, mat.c2r0 * scalar,
				mat.c0r1 * scalar, mat.c1r1 * scalar, mat.c2r1 * scalar,
				mat.c0r2 * scalar, mat.c1r2 * scalar, mat.c2r2 * scalar
			);
		}

		Matrix3x3& operator+=(const Matrix3x3& other) noexcept
		{
			c0r0 += other.c0r0; c1r0 += other.c1r0; c2r0 += other.c2r0;
			c0r1 += other.c0r1; c1r1 += other.c1r1; c2r1 += other.c2r1;
			c0r2 += other.c0r2; c1r2 += other.c1r2; c2r2 += other.c2r2;

			return *this;
		}
		Matrix3x3& operator-=(const Matrix3x3& other) noexcept
		{
			c0r0 -= other.c0r0; c1r0 -= other.c1r0; c2r0 -= other.c2r0;
			c0r1 -= other.c0r1; c1r1 -= other.c1r1; c2r1 -= other.c2r1;
			c0r2 -= other.c0r2; c1r2 -= other.c1r2; c2r2 -= other.c2r2;

			return *this;
		}
		Matrix3x3& operator*=(float scalar) noexcept
		{
			c0r0 *= scalar; c1r0 *= scalar; c2r0 *= scalar;
			c0r1 *= scalar; c1r1 *= scalar; c2r1 *= scalar;
			c0r2 *= scalar; c1r2 *= scalar; c2r2 *= scalar;

			return *this;
		}
		Matrix3x3& operator*=(const Matrix3x3& other) noexcept
		{
			*this = Matrix3x3(
				c0r0 * other.c0r0 + c1r0 * other.c0r1 + c2r0 * other.c0r2,
				c0r0 * other.c1r0 + c1r0 * other.c1r1 + c2r0 * other.c1r2,
				c0r0 * other.c2r0 + c1r0 * other.c2r1 + c2r0 * other.c2r2,
				c0r1 * other.c0r0 + c1r1 * other.c0r1 + c2r1 * other.c0r2,
				c0r1 * other.c1r0 + c1r1 * other.c1r1 + c2r1 * other.c1r2,
				c0r1 * other.c2r0 + c1r1 * other.c2r1 + c2r1 * other.c2r2,
				c0r2 * other.c0r0 + c1r2 * other.c0r1 + c2r2 * other.c0r2,
				c0r2 * other.c1r0 + c1r2 * other.c1r1 + c2r2 * other.c1r2,
				c0r2 * other.c2r0 + c1r2 * other.c2r1 + c2r2 * other.c2r2
			);

			return *this;
		}
		Matrix3x3& operator/=(float scalar) noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				*this = Matrix3x3::Zero();
			}
			else
			{
				c0r0 /= scalar; c1r0 /= scalar; c2r0 /= scalar;
				c0r1 /= scalar; c1r1 /= scalar; c2r1 /= scalar;
				c0r2 /= scalar; c1r2 /= scalar; c2r2 /= scalar;
			}

			return *this;
		}

		float Determinant() const noexcept
		{
			return
				c0r0 * (c1r1 * c2r2 - c2r1 * c1r2) -
				c1r0 * (c0r1 * c2r2 - c2r1 * c0r2) +
				c2r0 * (c0r1 * c1r2 - c1r1 * c0r2);
		}

		Matrix3x3 Transposed() const noexcept
		{
			return Matrix3x3(
				c0r0, c0r1, c0r2,
				c1r0, c1r1, c1r2,
				c2r0, c2r1, c2r2
			);
		}
		Matrix3x3& Transpose() noexcept
		{
			*this = Transposed();
			return *this;
		}
	};
}
