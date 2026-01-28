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
		// 定数

		static constexpr int RowSizeAlignment = 256; // アラインメントをこれに揃える必要がある
		static constexpr int MipLevels = 1;          // ミップマップなし

		// メンバ変数

		std::vector<std::uint8_t> data; // 生データ (ビット配列)
		Lattice3 size; // width, height, sliceCount
		Format format;

		// コンストラクタ

		Texture() : Texture({}, Lattice3::Zero(), Format::Unknown)
		{
		}
		Texture(const std::vector<std::uint8_t>& data, const Lattice3& size, Format format)
			: data(data), size(size), format(format)
		{
		}
		Texture(std::vector<std::uint8_t>&& data, const Lattice3& size, Format format)
			: data(std::move(data)), size(size), format(format)
		{
		}

		// メソッド

		int GetRowBytes() const { return GetFormatBytePerPixel(format) * size.x; }
		int GetSliceBytes() const { return GetRowBytes() * size.y; }
		int GetWholeBytes() const { return GetSliceBytes() * size.z; }

		bool IsValid() const
		{
			if (data.empty()) return false;
			if (size.x <= 0 || size.y <= 0 || size.z <= 0) return false;
			if (format == Format::Unknown) return false;

			return true;
		}
	};
}
