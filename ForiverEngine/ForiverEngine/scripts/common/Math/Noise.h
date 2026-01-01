#pragma once

#include <scripts/common/Math/Defines.h>

#include <oss/SimplexNoise.h>

namespace ForiverEngine
{
	class Noise final
	{
	public:
		Noise() = delete;
		~Noise() = delete;

		/// <summary>
		/// シンプレックスノイズ 1D
		/// </summary>
		/// <param name="x">X座標</param>
		/// <returns><para>[-1, 1]</para>格子点では常に 0</returns>
		static float Simplex1D(float x)
		{
			return SimplexNoise::noise(x);
		}

		/// <summary>
		/// シンプレックスノイズ 2D
		/// </summary>
		/// <param name="x">X座標</param>
		/// <param name="y">Y座標</param>
		/// <returns><para>[-1, 1]</para>格子点では常に 0</returns>
		static float Simplex2D(float x, float y)
		{
			return SimplexNoise::noise(x, y);
		}

		/// <summary>
		/// シンプレックスノイズ 3D
		/// </summary>
		/// <param name="x">X座標</param>
		/// <param name="y">Y座標</param>
		/// <param name="z">Z座標</param>
		/// <returns><para>[-1, 1]</para>格子点では常に 0</returns>
		static float Simplex3D(float x, float y, float z)
		{
			return SimplexNoise::noise(x, y, z);
		}
	};
}
