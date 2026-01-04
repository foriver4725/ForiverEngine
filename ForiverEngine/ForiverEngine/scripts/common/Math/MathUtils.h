#pragma once

#include <scripts/common/IncludeInternal.h>
#include <scripts/common/Math/Defines.h>

#include <concepts>

namespace ForiverEngine
{
	// 整数 or 浮動小数点数
	template<typename T>
	concept Numeric = std::integral<T> || std::floating_point<T>;

	class MathUtils final
	{
	public:
		DELETE_DEFAULT_METHODS(MathUtils);

		/// <summary>
		/// 数値が範囲内にあるか調べる. [begin, end)
		/// </summary>
		template<Numeric T>
		static constexpr bool IsInRange(T value, T begin, T end) noexcept
		{
			return begin <= value && value < end;
		}
	};
}
