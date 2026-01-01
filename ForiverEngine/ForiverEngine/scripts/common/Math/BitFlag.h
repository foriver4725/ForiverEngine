#pragma once

#include <scripts/common/IncludeInternal.h>
#include <scripts/common/Math/Defines.h>

#include <vector>
#include <concepts>

namespace ForiverEngine
{
	class BitFlag final
	{
	public:
		DELETE_DEFAULT_METHODS(BitFlag);

		/// <summary>
		/// <para>指定された桁のビットを立てる</para>
		/// 既に立っているなら、何も起こらない
		/// </summary>
		template<std::integral TBits>
		static constexpr TBits AddFlag(TBits bits, int digit) noexcept
		{
			return bits | (static_cast<TBits>(1) << digit);
		}

		/// <summary>
		/// <para>指定された桁のビットを立てる (複数を一気に)</para>
		/// 既に立っているなら、何も起こらない
		/// </summary>
		template<std::integral TBits>
		static constexpr TBits AddFlags(TBits bits, const std::vector<int>& digits) noexcept
		{
			for (int digit : digits)
				bits = AddFlag(bits, digit);
			return bits;
		}

		/// <summary>
		/// <para>指定された桁のビットを消す</para>
		/// 既に消えているなら、何も起こらない
		/// </summary>
		template<std::integral TBits>
		static constexpr TBits RemoveFlag(TBits bits, int digit) noexcept
		{
			return bits & ~(static_cast<TBits>(1) << digit);
		}

		/// <summary>
		/// <para>指定された桁のビットを消す (複数を一気に)</para>
		/// 既に消えているなら、何も起こらない
		/// </summary>
		template<std::integral TBits>
		static constexpr TBits RemoveFlags(TBits bits, const std::vector<int>& digits) noexcept
		{
			for (int digit : digits)
				bits = RemoveFlag(bits, digit);
			return bits;
		}

		/// <summary>
		/// <para>指定された桁のビットが立っているかを調べる</	para>
		/// </summary>
		template<std::integral TBits>
		static constexpr bool HasFlag(TBits bits, int digit) noexcept
		{
			return (bits & (static_cast<TBits>(1) << digit)) != 0;
		}
	};
}
