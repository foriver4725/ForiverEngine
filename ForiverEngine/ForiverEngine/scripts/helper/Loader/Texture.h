#pragma once

#include <scripts/common/Include.h>
#include "../headers/D3D12Defines.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>テクスチャ</para>
	/// <para>生データとそのメタデータ</para>
	/// </summary>
	struct Texture
	{
		// アラインメントをこれに揃える必要がある
		static constexpr int RowSizeAlignment = 256;

		std::vector<std::uint8_t> data{}; // 生データ (ビット配列 的な)

		GraphicsBufferType textureType{};
		Format format{};
		int width{};
		int height{};
		int rowSize{}; // 1行分のデータサイズ
		int sliceSize{}; // 1スライス分のデータサイズ
		int sliceCount{}; // スライス数
		int mipLevels{};

		constexpr bool IsValid() const { return !data.empty() && width > 0 && height > 0; }

		/// <summary>
		/// <para>手動作成</para>
		/// <para>2Dテクスチャ (配列ではない) として作成する</para>
		/// <para>生データはそのまま素通しし、そこから値を計算などはしない</para>
		/// <para>ミップマップなし</para>
		/// </summary>
		static Texture CreateManually(const std::vector<std::uint8_t>& data, const Lattice2& size, Format format)
		{
			// 1テクセルのバイト数を計算

			const std::uint32_t formatTypes = GetFormatTypes(format);

			int channelAmount = 0;
			int biteAmountPerChannel = 0;
			{
				if (BitFlag::HasFlag(formatTypes, FormatTypeDigit::Dim1))
					channelAmount = 1;
				else if (BitFlag::HasFlag(formatTypes, FormatTypeDigit::Dim2))
					channelAmount = 2;
				else if (BitFlag::HasFlag(formatTypes, FormatTypeDigit::Dim3))
					channelAmount = 3;
				else if (BitFlag::HasFlag(formatTypes, FormatTypeDigit::Dim4))
					channelAmount = 4;
				else
					channelAmount = 0; // 不明

				if (BitFlag::HasFlag(formatTypes, FormatTypeDigit::Bite1))
					biteAmountPerChannel = 1;
				else if (BitFlag::HasFlag(formatTypes, FormatTypeDigit::Bite2))
					biteAmountPerChannel = 2;
				else if (BitFlag::HasFlag(formatTypes, FormatTypeDigit::Bite4))
					biteAmountPerChannel = 4;
				else
					biteAmountPerChannel = 0; // 不明
			}

			const std::size_t biteAmountTotal = static_cast<std::size_t>(channelAmount * biteAmountPerChannel);

			return Texture
			{
				.data = data,
				.textureType = GraphicsBufferType::Texture2D,
				.format = format,
				.width = size.x,
				.height = size.y,
				.rowSize = static_cast<int>(biteAmountTotal * size.x),
				.sliceSize = static_cast<int>(biteAmountTotal * size.x * size.y),
				.sliceCount = 1,
				.mipLevels = 1,
			};
		}
	};
}
