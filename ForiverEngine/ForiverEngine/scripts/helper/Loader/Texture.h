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

		// 決め打ち. ミップマップなし
		static constexpr int MipLevels = 1;

		std::vector<std::uint8_t> data = {}; // 生データ (ビット配列)

		int width = 0;
		int height = 0;
		int sliceCount = 0; // スライス数

		// 基本的に決め打ち. RGBA 8bit
		// RT, SR などを作成する時のメタデータとして使いたい場合、違う値に上書きするケースもある
		Format format = Format::RGBA_U8;

		constexpr int GetRowSize() const { return GetFormatTotalBytes(format) * width; }
		constexpr int GetSliceSize() const { return GetRowSize() * height; }
		constexpr int GetWholeSize() const { return GetSliceSize() * sliceCount; }

		constexpr bool IsValid() const { return !data.empty() && width > 0 && height > 0 && sliceCount > 0; }
	};
}
