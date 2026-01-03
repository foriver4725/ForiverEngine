#pragma once

#include <scripts/common/IncludeInternal.h>

#include <memory>

namespace ForiverEngine
{
	/// <summary>
	/// ヒープ上に多次元配列を確保するためのユーティリティクラス
	/// </summary>
	class HeapMultiDimAllocator final
	{
	public:
		DELETE_DEFAULT_METHODS(HeapMultiDimAllocator);

		template<typename T>
		using Array1D = std::unique_ptr<T[]>;

		template<typename T>
		using Array2D = std::unique_ptr<Array1D<T>[]>;

		template<typename T>
		using Array3D = std::unique_ptr<Array2D<T>[]>;

		template<typename T>
		static Array1D<T> CreateArray1D(int size)
		{
			// RVO に任せる. move しない
			return std::make_unique<T[]>(size);
		}

		// x,y の順でアクセス
		template<typename T>
		static Array2D<T> CreateArray2D(int sizeX, int sizeY)
		{
			auto array2D = std::make_unique<Array1D<T>[]>(sizeX);
			for (int i = 0; i < sizeX; ++i)
				array2D[i] = CreateArray1D<T>(sizeY);

			// RVO に任せる. move しない
			return array2D;
		}

		// x,y,z の順でアクセス
		template <typename T>
		static Array3D<T> CreateArray3D(int sizeX, int sizeY, int sizeZ)
		{
			auto array3D = std::make_unique<Array2D<T>[]>(sizeX);
			for (int i = 0; i < sizeX; ++i)
				array3D[i] = CreateArray2D<T>(sizeY, sizeZ);

			// RVO に任せる. move しない
			return array3D;
		}
	};
}
