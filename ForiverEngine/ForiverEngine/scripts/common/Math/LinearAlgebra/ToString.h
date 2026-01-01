#pragma once

#include "./headers/Lattice2.h"
#include "./headers/Lattice3.h"
#include "./headers/Lattice4.h"
#include "./headers/Vector2.h"
#include "./headers/Vector3.h"
#include "./headers/Vector4.h"
#include "./headers/Matrix2x2.h"
#include "./headers/Matrix3x3.h"
#include "./headers/Matrix4x4.h"
#include "./headers/Quaternion.h"

#include <string>
#include <format>

namespace ForiverEngine
{
	inline std::string ToString(const Lattice2& lattice)
	{
		return std::format("({},{})", lattice.x, lattice.y);
	}

	inline std::string ToString(const Lattice3& lattice)
	{
		return std::format("({},{},{})", lattice.x, lattice.y, lattice.z);
	}

	inline std::string ToString(const Lattice4& lattice)
	{
		return std::format("({},{},{},{})", lattice.x, lattice.y, lattice.z, lattice.w);
	}

	inline std::string ToString(const Vector2& vec)
	{
		return std::format("({:.2f},{:.2f})", vec.x, vec.y);
	}

	inline std::string ToString(const Vector3& vec)
	{
		return std::format("({:.2f},{:.2f},{:.2f})", vec.x, vec.y, vec.z);
	}

	inline std::string ToString(const Vector4& vec)
	{
		return std::format("({:.2f},{:.2f},{:.2f},{:.2f})", vec.x, vec.y, vec.z, vec.w);
	}

	inline std::string ToString(const Matrix2x2& mat)
	{
		return std::format(
			"[[{:.2f},{:.2f}],[{:.2f},{:.2f}]]",
			mat.c0r0, mat.c1r0,
			mat.c0r1, mat.c1r1
		);
	}

	inline std::string ToString(const Matrix3x3& mat)
	{
		return std::format(
			"[[{:.2f},{:.2f},{:.2f}],[{:.2f},{:.2f},{:.2f}],[{:.2f},{:.2f},{:.2f}]]",
			mat.c0r0, mat.c1r0, mat.c2r0,
			mat.c0r1, mat.c1r1, mat.c2r1,
			mat.c0r2, mat.c1r2, mat.c2r2
		);
	}

	inline std::string ToString(const Matrix4x4& mat)
	{
		return std::format(
			"[[{:.2f},{:.2f},{:.2f},{:.2f}],[{:.2f},{:.2f},{:.2f},{:.2f}],[{:.2f},{:.2f},{:.2f},{:.2f}],[{:.2f},{:.2f},{:.2f},{:.2f}]]",
			mat.c0r0, mat.c1r0, mat.c2r0, mat.c3r0,
			mat.c0r1, mat.c1r1, mat.c2r1, mat.c3r1,
			mat.c0r2, mat.c1r2, mat.c2r2, mat.c3r2,
			mat.c0r3, mat.c1r3, mat.c2r3, mat.c3r3
		);
	}

	inline std::string ToString(const Quaternion& quat)
	{
		return std::format("({:.2f},{:.2f},{:.2f},{:.2f})", quat.x, quat.y, quat.z, quat.w);
	}
}
