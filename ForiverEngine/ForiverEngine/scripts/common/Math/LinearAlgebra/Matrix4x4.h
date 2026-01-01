#pragma once

#include <scripts/common/IncludeInternal.h>
#include <scripts/common/Math/Defines.h>

#include "./Vector4.h"
#include "./Quaternion.h"
#include "./Matrix3x3.h"

namespace ForiverEngine
{
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
		constexpr Matrix4x4(const Matrix3x3& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(other.c2r0), c3r0(0.0f)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(other.c2r1), c3r1(0.0f)
			, c0r2(other.c0r2), c1r2(other.c1r2), c2r2(other.c2r2), c3r2(0.0f)
			, c0r3(0.0f), c1r3(0.0f), c2r3(0.0f), c3r3(1.0f)
		{
		}
		constexpr Matrix4x4(Matrix3x3&& other) noexcept
			: c0r0(other.c0r0), c1r0(other.c1r0), c2r0(other.c2r0), c3r0(0.0f)
			, c0r1(other.c0r1), c1r1(other.c1r1), c2r1(other.c2r1), c3r1(0.0f)
			, c0r2(other.c0r2), c1r2(other.c1r2), c2r2(other.c2r2), c3r2(0.0f)
			, c0r3(0.0f), c1r3(0.0f), c2r3(0.0f), c3r3(1.0f)
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

		static constexpr Matrix4x4 FromColumnVectors(
			const Vector4& c0,
			const Vector4& c1,
			const Vector4& c2,
			const Vector4& c3
		) noexcept
		{
			return Matrix4x4(
				c0.x, c1.x, c2.x, c3.x,
				c0.y, c1.y, c2.y, c3.y,
				c0.z, c1.z, c2.z, c3.z,
				c0.w, c1.w, c2.w, c3.w
			);
		}
		static constexpr Matrix4x4 FromRowVectors(
			const Vector4& r0,
			const Vector4& r1,
			const Vector4& r2,
			const Vector4& r3
		) noexcept
		{
			return Matrix4x4(
				r0.x, r0.y, r0.z, r0.w,
				r1.x, r1.y, r1.z, r1.w,
				r2.x, r2.y, r2.z, r2.w,
				r3.x, r3.y, r3.z, r3.w
			);
		}

		static Matrix4x4 Scale(float sx, float sy, float sz) noexcept
		{
			const Matrix3x3 scale3x3 = Matrix3x3::Scale(sx, sy, sz);
			return Matrix4x4(scale3x3);
		};
		static Matrix4x4 Scale(const Vector3& scale) noexcept
		{
			return Scale(scale.x, scale.y, scale.z);
		}
		static Matrix4x4 Rotate(Quaternion quat) noexcept
		{
			const Matrix3x3 rot3x3 = Matrix3x3::Rotate(quat);
			return Matrix4x4(rot3x3);
		}
		static Matrix4x4 Translate(float tx, float ty, float tz) noexcept
		{
			return Matrix4x4(
				1.0f, 0.0f, 0.0f, tx,
				0.0f, 1.0f, 0.0f, ty,
				0.0f, 0.0f, 1.0f, tz,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}
		static Matrix4x4 Translate(const Vector3& translation) noexcept
		{
			return Translate(translation.x, translation.y, translation.z);
		}

		Matrix4x4& operator=(const Matrix4x4& other) noexcept
		{
			if (this != &other)
			{
				c0r0 = other.c0r0; c1r0 = other.c1r0; c2r0 = other.c2r0; c3r0 = other.c3r0;
				c0r1 = other.c0r1; c1r1 = other.c1r1; c2r1 = other.c2r1; c3r1 = other.c3r1;
				c0r2 = other.c0r2; c1r2 = other.c1r2; c2r2 = other.c2r2; c3r2 = other.c3r2;
				c0r3 = other.c0r3; c1r3 = other.c1r3; c2r3 = other.c2r3; c3r3 = other.c3r3;
			}
			return *this;
		}
		Matrix4x4& operator=(Matrix4x4&& other) noexcept
		{
			if (this != &other)
			{
				c0r0 = other.c0r0; c1r0 = other.c1r0; c2r0 = other.c2r0; c3r0 = other.c3r0;
				c0r1 = other.c0r1; c1r1 = other.c1r1; c2r1 = other.c2r1; c3r1 = other.c3r1;
				c0r2 = other.c0r2; c1r2 = other.c1r2; c2r2 = other.c2r2; c3r2 = other.c3r2;
				c0r3 = other.c0r3; c1r3 = other.c1r3; c2r3 = other.c2r3; c3r3 = other.c3r3;
			}
			return *this;
		}

		bool operator==(const Matrix4x4& other) const noexcept
		{
			return std::abs(c0r0 - other.c0r0) < Epsilon && std::abs(c1r0 - other.c1r0) < Epsilon && std::abs(c2r0 - other.c2r0) < Epsilon && std::abs(c3r0 - other.c3r0) < Epsilon
				&& std::abs(c0r1 - other.c0r1) < Epsilon && std::abs(c1r1 - other.c1r1) < Epsilon && std::abs(c2r1 - other.c2r1) < Epsilon && std::abs(c3r1 - other.c3r1) < Epsilon
				&& std::abs(c0r2 - other.c0r2) < Epsilon && std::abs(c1r2 - other.c1r2) < Epsilon && std::abs(c2r2 - other.c2r2) < Epsilon && std::abs(c3r2 - other.c3r2) < Epsilon
				&& std::abs(c0r3 - other.c0r3) < Epsilon && std::abs(c1r3 - other.c1r3) < Epsilon && std::abs(c2r3 - other.c2r3) < Epsilon && std::abs(c3r3 - other.c3r3) < Epsilon;
		}
		bool operator!=(const Matrix4x4& other) const noexcept
		{
			return !(*this == other);
		}

		Matrix4x4 operator+() const noexcept
		{
			return *this;
		}
		Matrix4x4 operator-() const noexcept
		{
			return Matrix4x4(
				-c0r0, -c1r0, -c2r0, -c3r0,
				-c0r1, -c1r1, -c2r1, -c3r1,
				-c0r2, -c1r2, -c2r2, -c3r2,
				-c0r3, -c1r3, -c2r3, -c3r3
			);
		}

		Matrix4x4 operator+(const Matrix4x4& other) const noexcept
		{
			return Matrix4x4(
				c0r0 + other.c0r0, c1r0 + other.c1r0, c2r0 + other.c2r0, c3r0 + other.c3r0,
				c0r1 + other.c0r1, c1r1 + other.c1r1, c2r1 + other.c2r1, c3r1 + other.c3r1,
				c0r2 + other.c0r2, c1r2 + other.c1r2, c2r2 + other.c2r2, c3r2 + other.c3r2,
				c0r3 + other.c0r3, c1r3 + other.c1r3, c2r3 + other.c2r3, c3r3 + other.c3r3
			);
		}
		Matrix4x4 operator-(const Matrix4x4& other) const noexcept
		{
			return Matrix4x4(
				c0r0 - other.c0r0, c1r0 - other.c1r0, c2r0 - other.c2r0, c3r0 - other.c3r0,
				c0r1 - other.c0r1, c1r1 - other.c1r1, c2r1 - other.c2r1, c3r1 - other.c3r1,
				c0r2 - other.c0r2, c1r2 - other.c1r2, c2r2 - other.c2r2, c3r2 - other.c3r2,
				c0r3 - other.c0r3, c1r3 - other.c1r3, c2r3 - other.c2r3, c3r3 - other.c3r3
			);
		}
		Matrix4x4 operator*(float scalar) const noexcept
		{
			return Matrix4x4(
				c0r0 * scalar, c1r0 * scalar, c2r0 * scalar, c3r0 * scalar,
				c0r1 * scalar, c1r1 * scalar, c2r1 * scalar, c3r1 * scalar,
				c0r2 * scalar, c1r2 * scalar, c2r2 * scalar, c3r2 * scalar,
				c0r3 * scalar, c1r3 * scalar, c2r3 * scalar, c3r3 * scalar
			);
		}
		Matrix4x4 operator*(const Matrix4x4& other) const noexcept
		{
			return Matrix4x4(
				c0r0 * other.c0r0 + c1r0 * other.c0r1 + c2r0 * other.c0r2 + c3r0 * other.c0r3,
				c0r0 * other.c1r0 + c1r0 * other.c1r1 + c2r0 * other.c1r2 + c3r0 * other.c1r3,
				c0r0 * other.c2r0 + c1r0 * other.c2r1 + c2r0 * other.c2r2 + c3r0 * other.c2r3,
				c0r0 * other.c3r0 + c1r0 * other.c3r1 + c2r0 * other.c3r2 + c3r0 * other.c3r3,
				c0r1 * other.c0r0 + c1r1 * other.c0r1 + c2r1 * other.c0r2 + c3r1 * other.c0r3,
				c0r1 * other.c1r0 + c1r1 * other.c1r1 + c2r1 * other.c1r2 + c3r1 * other.c1r3,
				c0r1 * other.c2r0 + c1r1 * other.c2r1 + c2r1 * other.c2r2 + c3r1 * other.c2r3,
				c0r1 * other.c3r0 + c1r1 * other.c3r1 + c2r1 * other.c3r2 + c3r1 * other.c3r3,
				c0r2 * other.c0r0 + c1r2 * other.c0r1 + c2r2 * other.c0r2 + c3r2 * other.c0r3,
				c0r2 * other.c1r0 + c1r2 * other.c1r1 + c2r2 * other.c1r2 + c3r2 * other.c1r3,
				c0r2 * other.c2r0 + c1r2 * other.c2r1 + c2r2 * other.c2r2 + c3r2 * other.c2r3,
				c0r2 * other.c3r0 + c1r2 * other.c3r1 + c2r2 * other.c3r2 + c3r2 * other.c3r3,
				c0r3 * other.c0r0 + c1r3 * other.c0r1 + c2r3 * other.c0r2 + c3r3 * other.c0r3,
				c0r3 * other.c1r0 + c1r3 * other.c1r1 + c2r3 * other.c1r2 + c3r3 * other.c1r3,
				c0r3 * other.c2r0 + c1r3 * other.c2r1 + c2r3 * other.c2r2 + c3r3 * other.c2r3,
				c0r3 * other.c3r0 + c1r3 * other.c3r1 + c2r3 * other.c3r2 + c3r3 * other.c3r3
			);
		}
		Vector4 operator*(const Vector4& vec) const noexcept
		{
			return Vector4(
				c0r0 * vec.x + c1r0 * vec.y + c2r0 * vec.z + c3r0 * vec.w,
				c0r1 * vec.x + c1r1 * vec.y + c2r1 * vec.z + c3r1 * vec.w,
				c0r2 * vec.x + c1r2 * vec.y + c2r2 * vec.z + c3r2 * vec.w,
				c0r3 * vec.x + c1r3 * vec.y + c2r3 * vec.z + c3r3 * vec.w
			);
		}
		Matrix4x4 operator/(float scalar) const noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				return Matrix4x4::Zero();
			}
			return Matrix4x4(
				c0r0 / scalar, c1r0 / scalar, c2r0 / scalar, c3r0 / scalar,
				c0r1 / scalar, c1r1 / scalar, c2r1 / scalar, c3r1 / scalar,
				c0r2 / scalar, c1r2 / scalar, c2r2 / scalar, c3r2 / scalar,
				c0r3 / scalar, c1r3 / scalar, c2r3 / scalar, c3r3 / scalar
			);
		}
		friend Matrix4x4 operator*(float scalar, const Matrix4x4& mat) noexcept
		{
			return Matrix4x4(
				mat.c0r0 * scalar, mat.c1r0 * scalar, mat.c2r0 * scalar, mat.c3r0 * scalar,
				mat.c0r1 * scalar, mat.c1r1 * scalar, mat.c2r1 * scalar, mat.c3r1 * scalar,
				mat.c0r2 * scalar, mat.c1r2 * scalar, mat.c2r2 * scalar, mat.c3r2 * scalar,
				mat.c0r3 * scalar, mat.c1r3 * scalar, mat.c2r3 * scalar, mat.c3r3 * scalar
			);
		}

		Matrix4x4& operator+=(const Matrix4x4& other) noexcept
		{
			c0r0 += other.c0r0; c1r0 += other.c1r0; c2r0 += other.c2r0; c3r0 += other.c3r0;
			c0r1 += other.c0r1; c1r1 += other.c1r1; c2r1 += other.c2r1; c3r1 += other.c3r1;
			c0r2 += other.c0r2; c1r2 += other.c1r2; c2r2 += other.c2r2; c3r2 += other.c3r2;
			c0r3 += other.c0r3; c1r3 += other.c1r3; c2r3 += other.c2r3; c3r3 += other.c3r3;

			return *this;
		}
		Matrix4x4& operator-=(const Matrix4x4& other) noexcept
		{
			c0r0 -= other.c0r0; c1r0 -= other.c1r0; c2r0 -= other.c2r0; c3r0 -= other.c3r0;
			c0r1 -= other.c0r1; c1r1 -= other.c1r1; c2r1 -= other.c2r1; c3r1 -= other.c3r1;
			c0r2 -= other.c0r2; c1r2 -= other.c1r2; c2r2 -= other.c2r2; c3r2 -= other.c3r2;
			c0r3 -= other.c0r3; c1r3 -= other.c1r3; c2r3 -= other.c2r3; c3r3 -= other.c3r3;

			return *this;
		}
		Matrix4x4& operator*=(float scalar) noexcept
		{
			c0r0 *= scalar; c1r0 *= scalar; c2r0 *= scalar; c3r0 *= scalar;
			c0r1 *= scalar; c1r1 *= scalar; c2r1 *= scalar; c3r1 *= scalar;
			c0r2 *= scalar; c1r2 *= scalar; c2r2 *= scalar; c3r2 *= scalar;
			c0r3 *= scalar; c1r3 *= scalar; c2r3 *= scalar; c3r3 *= scalar;

			return *this;
		}
		Matrix4x4& operator*=(const Matrix4x4& other) noexcept
		{
			*this = Matrix4x4(
				c0r0 * other.c0r0 + c1r0 * other.c0r1 + c2r0 * other.c0r2 + c3r0 * other.c0r3,
				c0r0 * other.c1r0 + c1r0 * other.c1r1 + c2r0 * other.c1r2 + c3r0 * other.c1r3,
				c0r0 * other.c2r0 + c1r0 * other.c2r1 + c2r0 * other.c2r2 + c3r0 * other.c2r3,
				c0r0 * other.c3r0 + c1r0 * other.c3r1 + c2r0 * other.c3r2 + c3r0 * other.c3r3,
				c0r1 * other.c0r0 + c1r1 * other.c0r1 + c2r1 * other.c0r2 + c3r1 * other.c0r3,
				c0r1 * other.c1r0 + c1r1 * other.c1r1 + c2r1 * other.c1r2 + c3r1 * other.c1r3,
				c0r1 * other.c2r0 + c1r1 * other.c2r1 + c2r1 * other.c2r2 + c3r1 * other.c2r3,
				c0r1 * other.c3r0 + c1r1 * other.c3r1 + c2r1 * other.c3r2 + c3r1 * other.c3r3,
				c0r2 * other.c0r0 + c1r2 * other.c0r1 + c2r2 * other.c0r2 + c3r2 * other.c0r3,
				c0r2 * other.c1r0 + c1r2 * other.c1r1 + c2r2 * other.c1r2 + c3r2 * other.c1r3,
				c0r2 * other.c2r0 + c1r2 * other.c2r1 + c2r2 * other.c2r2 + c3r2 * other.c2r3,
				c0r2 * other.c3r0 + c1r2 * other.c3r1 + c2r2 * other.c3r2 + c3r2 * other.c3r3,
				c0r3 * other.c0r0 + c1r3 * other.c0r1 + c2r3 * other.c0r2 + c3r3 * other.c0r3,
				c0r3 * other.c1r0 + c1r3 * other.c1r1 + c2r3 * other.c1r2 + c3r3 * other.c1r3,
				c0r3 * other.c2r0 + c1r3 * other.c2r1 + c2r3 * other.c2r2 + c3r3 * other.c2r3,
				c0r3 * other.c3r0 + c1r3 * other.c3r1 + c2r3 * other.c3r2 + c3r3 * other.c3r3
			);

			return *this;
		}
		Matrix4x4& operator/=(float scalar) noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				*this = Matrix4x4::Zero();
			}
			else
			{
				c0r0 /= scalar; c1r0 /= scalar; c2r0 /= scalar; c3r0 /= scalar;
				c0r1 /= scalar; c1r1 /= scalar; c2r1 /= scalar; c3r1 /= scalar;
				c0r2 /= scalar; c1r2 /= scalar; c2r2 /= scalar; c3r2 /= scalar;
				c0r3 /= scalar; c1r3 /= scalar; c2r3 /= scalar; c3r3 /= scalar;
			}

			return *this;
		}

		float Determinant() const noexcept
		{
			const float subFactor00 = c2r2 * c3r3 - c3r2 * c2r3;
			const float subFactor01 = c2r1 * c3r3 - c3r1 * c2r3;
			const float subFactor02 = c2r1 * c3r2 - c3r1 * c2r2;
			const float subFactor03 = c2r0 * c3r3 - c3r0 * c2r3;
			const float subFactor04 = c2r0 * c3r2 - c3r0 * c2r2;
			const float subFactor05 = c2r0 * c3r1 - c3r0 * c2r1;

			const Vector4 detCof(
				+(c1r1 * subFactor00 - c1r2 * subFactor01 + c1r3 * subFactor02),
				-(c1r0 * subFactor00 - c1r2 * subFactor03 + c1r3 * subFactor04),
				+(c1r0 * subFactor01 - c1r1 * subFactor03 + c1r3 * subFactor05),
				-(c1r0 * subFactor02 - c1r1 * subFactor04 + c1r2 * subFactor05)
			);

			return
				c0r0 * detCof.x +
				c0r1 * detCof.y +
				c0r2 * detCof.z +
				c0r3 * detCof.w;
		}
		Matrix4x4 Transposed() const noexcept
		{
			return Matrix4x4(
				c0r0, c0r1, c0r2, c0r3,
				c1r0, c1r1, c1r2, c1r3,
				c2r0, c2r1, c2r2, c2r3,
				c3r0, c3r1, c3r2, c3r3
			);
		}
		Matrix4x4& Transpose() noexcept
		{
			*this = Transposed();
			return *this;
		}
	};
}
