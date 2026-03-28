#pragma once

#include "scripts/gameFlow/IncludeInternal.h"

namespace ForiverEngine
{
	class WorldDataSaveLoadManager final
	{
	public:
		DELETE_DEFAULT_METHODS(WorldDataSaveLoadManager);

		inline static const std::filesystem::path WorldSaveDataDirectory = std::filesystem::path("saves") / "world";
		inline static const std::string WorldSaveDataExtension = ".world";

		inline static const std::filesystem::path WorldNameFilePath = std::filesystem::current_path() / "WorldName.txt";
		inline static const std::string DefaultWorldName = "NewWorld";

	private:
		// ワールド名 -> ワールドセーブデータのパス
		static std::filesystem::path GetWorldSaveDataPath(const std::string& worldName)
		{
			return WorldSaveDataDirectory / (worldName + WorldSaveDataExtension);
		}

	public:
		/// <summary>
		/// ワールドセーブデータが存在するかチェックする
		/// </summary>
		static bool Exists(const std::string& worldName)
		{
			return std::filesystem::exists(GetWorldSaveDataPath(worldName));
		}

		/// <summary>
		/// ワールドデータ(バイナリ)をセーブする
		/// </summary>
		/// <returns>成功したら true, 失敗したら false</returns>
		static bool Save(const std::string& worldName, const std::string& binary)
		{
			std::ofstream ofs(GetWorldSaveDataPath(worldName), std::ios::binary);
			if (!ofs) return false;

			ofs.write(binary.data(), static_cast<std::streamsize>(binary.size()));
			return ofs.good();
		}

		/// <summary>
		/// ワールドデータ(バイナリ)をロードする
		/// </summary>
		/// <returns>成功したら true, 失敗したら false</returns>
		static bool Load(const std::string& worldName, std::string& outBinary)
		{
			std::ifstream ifs(GetWorldSaveDataPath(worldName), std::ios::binary);
			if (!ifs) return false;

			ifs.seekg(0, std::ios::end);
			const std::streamsize size = ifs.tellg();
			ifs.seekg(0, std::ios::beg);

			outBinary.resize(static_cast<std::size_t>(size));
			ifs.read(outBinary.data(), size);

			return ifs.good();
		}

		/// <summary>
		/// ワールド名をファイルからロードして取得する
		/// </summary>
		static std::string LoadWorldName()
		{
			std::ifstream ifs(WorldNameFilePath);
			if (!ifs)
			{
				ShowError(L"ワールド名ファイルの読み込みに失敗しました");
				return DefaultWorldName;
			}

			std::string worldName;
			std::getline(ifs, worldName);

			// 一応改行文字をトリムしておく
			worldName.erase(worldName.find_last_not_of(" \n\r\t") + 1);

			return worldName;
		}

		// [ワールドデータ(バイナリ)のフォーマット]
		//
		// [プレイヤーTransformのサイズ]
		// ... プレイヤーTransformのバイナリデータ ...
		// [地形データのサイズ]
		// ... 地形データのバイナリデータ ...

		/// <summary>
		/// 必要なバイナリデータを結合して、ワールドデータ(バイナリ)を作成する
		/// </summary>
		static std::string CombineWorldDataBinaries(
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

		/// <summary>
		/// ワールドデータ(バイナリ)を分割して、個別のバイナリデータに分ける
		/// </summary>
		static std::tuple<std::string_view, std::string_view> SplitWorldDataBinaries(std::string_view combinedBinary)
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
