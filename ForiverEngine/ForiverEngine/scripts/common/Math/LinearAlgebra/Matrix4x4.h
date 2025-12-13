#pragma once

#include "./Defines.h"

namespace ForiverEngine
{
	// 列優先
	struct Matrix4x4
	{
		float c0r0; float c1r0; float c2r0; float c3r0;
		float c0r1; float c1r1; float c2r1; float c3r1;
		float c0r2; float c1r2; float c2r2; float c3r2;
		float c0r3; float c1r3; float c2r3; float c3r3;

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
	};
}
