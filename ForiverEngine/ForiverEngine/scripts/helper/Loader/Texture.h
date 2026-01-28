#pragma once

#include <scripts/common/Include.h>
#include "../headers/D3D12Defines.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>2Dテクスチャ/2Dテクスチャ配列</para>
	/// <para>生データとそのメタデータ</para>
	/// </summary>
	struct Texture
	{
		// アラインメントをこれに揃える必要がある
		static constexpr int RowSizeAlignment = 256;
		// ミップマップなし
		static constexpr int MipLevels = 1;

		std::vector<std::uint8_t> data{}; // 生データ (ビット配列)

		Format format{};
		Lattice2 size{};
		int rowSize{}; // 1行分のデータサイズ
		int sliceSize{}; // 1スライス分のデータサイズ
		int sliceCount{}; // スライス数

		constexpr bool IsValid() const { return !data.empty() && size.x > 0 && size.y > 0 && sliceCount > 0; }

		/// <summary>
		/// <para>手動作成</para>
		/// <para>2Dテクスチャ (配列ではない) として作成する</para>
		/// <para>生データはそのまま素通しし、そこから値を計算などはしない</para>
		/// <para>ミップマップなし</para>
		/// </summary>
		static Texture CreateManually(const std::vector<std::uint8_t>& data, const Lattice2& size, Format format)
		{
			const int bytePerPixel = GetFormatBytePerPixel(format);

			return Texture
			{
				.data = data,
				.format = format,
				.size = size,
				.rowSize = bytePerPixel * size.x,
				.sliceSize = bytePerPixel * size.x * size.y,
				.sliceCount = 1,
			};
		}
	};
}
