#pragma once

#include "scripts/gameFlow/IncludeInternal.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>ワールドデータを管理する</para>
	/// <para>セーブ・ロード用</para>
	/// </summary>
	class WorldSaveDataManager final
	{
	public:
		DELETE_DEFAULT_METHODS(WorldSaveDataManager);

		// [バイナリデータのフォーマット]
		//
		// [プレイヤーTransformのサイズ]
		// ... プレイヤーTransformのバイナリデータ ...
		// [地形データのサイズ]
		// ... 地形データのバイナリデータ ...

		static std::string CombineBinaries(
			std::string_view playerTransformBinary,
			std::string_view terrainBinary
		)
		{
			std::string buffer;

			const std::uint64_t playerTransformBinarySize = static_cast<std::uint64_t>(playerTransformBinary.size());
			const std::uint64_t terrainBinarySize = static_cast<std::uint64_t>(terrainBinary.size());

			buffer.append(reinterpret_cast<const char*>(&playerTransformBinarySize), sizeof(playerTransformBinarySize));
			buffer.append(playerTransformBinary.data(), playerTransformBinary.size());
			buffer.append(reinterpret_cast<const char*>(&terrainBinarySize), sizeof(terrainBinarySize));
			buffer.append(terrainBinary.data(), terrainBinary.size());

			return buffer;
		}

		static std::tuple<std::string_view, std::string_view> SplitBinaries(std::string_view combinedBinary)
		{
			const char* ptr = combinedBinary.data();
			const char* end = ptr + combinedBinary.size();

			auto read = [&](void* dst, std::size_t size) -> bool
				{
					if (ptr + size > end)
					{
						return false;
					}

					std::memcpy(dst, ptr, size);
					ptr += size;

					return true;
				};

			std::uint64_t playerTransformBinarySize = 0;
			std::uint64_t terrainBinarySize = 0;

			if (!read(&playerTransformBinarySize, sizeof(playerTransformBinarySize)))
			{
				return { {}, {} };
			}

			if (ptr + playerTransformBinarySize > end)
			{
				return { {}, {} };
			}
			std::string_view playerTransformBinary(ptr, static_cast<std::size_t>(playerTransformBinarySize));
			ptr += playerTransformBinarySize;

			if (!read(&terrainBinarySize, sizeof(terrainBinarySize)))
			{
				return { {}, {} };
			}

			if (ptr + terrainBinarySize > end)
			{
				return { {}, {} };
			}
			std::string_view terrainBinary(ptr, static_cast<std::size_t>(terrainBinarySize));
			ptr += terrainBinarySize;

			return { playerTransformBinary, terrainBinary };
		}
	};
}
