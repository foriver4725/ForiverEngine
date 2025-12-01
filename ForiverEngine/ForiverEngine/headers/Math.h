#pragma once

namespace ForiverEngine
{
	class Math final
	{
	public:
		Math() = delete;
		~Math() = delete;

		/// <summary>
		/// シンプレックスノイズ 1D
		/// </summary>
		/// <param name="x">X座標</param>
		/// <returns><para>[-1, 1]</para>格子点では常に 0</returns>
		static float SimplexNoise1D(float x);

		/// <summary>
		/// シンプレックスノイズ 2D
		/// </summary>
		/// <param name="x">X座標</param>
		/// <param name="y">Y座標</param>
		/// <returns><para>[-1, 1]</para>格子点では常に 0</returns>
		static float SimplexNoise2D(float x, float y);

		/// <summary>
		/// シンプレックスノイズ 3D
		/// </summary>
		/// <param name="x">X座標</param>
		/// <param name="y">Y座標</param>
		/// <param name="z">Z座標</param>
		/// <returns><para>[-1, 1]</para>格子点では常に 0</returns>
		static float SimplexNoise3D(float x, float y, float z);
	};
}
