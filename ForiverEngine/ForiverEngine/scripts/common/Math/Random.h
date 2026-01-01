#pragma once

#include <scripts/common/IncludeInternal.h>
#include <scripts/common/Math/Defines.h>

#include <random>
#include <cstdint>
#include <concepts>

namespace ForiverEngine
{
	class Random final
	{
	public:
		DELETE_DEFAULT_METHODS(Random);

		/// <summary>
		/// シード値を変更 (呼び出されたスレッドでのみ有効)
		/// </summary>
		static void SetSeed(std::uint32_t seed) { engine.seed(static_cast<unsigned int>(seed)); }

		/// <summary>
		/// 乱数生成. [min, max] の整数
		/// </summary>
		template <std::integral TInt>
		static TInt Range(TInt min, TInt max) { return std::uniform_int_distribution<TInt>{min, max}(engine); }

		/// <summary>
		/// 乱数生成. [min, max) の実数
		/// </summary>
		template <std::floating_point TFloat>
		static TFloat Range(TFloat min, TFloat max) { return std::uniform_real_distribution<TFloat>{min, max}(engine); }

	private:
		// 乱数生成器
		inline static thread_local std::mt19937 engine = std::mt19937(std::random_device{}());
	};
}
