#pragma once

#include "scripts/gameFlow/IncludeInternal.h"

namespace ForiverEngine
{
	class SaveLoadManager final
	{
	public:
		DELETE_DEFAULT_METHODS(SaveLoadManager);

		inline static const std::filesystem::path SaveDataDirectory = std::filesystem::path("saves");

		/// <summary>
		/// セーブデータが存在するかチェックする
		/// </summary>
		/// <param name="path">セーブデータのパス : セーブデータディレクトリからの相対パス
		/// <returns>存在するなら true, 存在しないなら false</returns>
		static bool Exists(const std::filesystem::path& path)
		{
			return std::filesystem::exists(SaveDataDirectory / path);
		}

		/// <summary>
		/// バイナリデータをセーブする
		/// </summary>
		/// <param name="path">セーブ先 : セーブデータディレクトリからの相対パス</param>
		/// <param name="binary">セーブするバイナリデータ</param>
		/// <returns>成功したら true, 失敗したら false</returns>
		static bool Save(const std::filesystem::path& path, const std::string& binary)
		{
			std::ofstream ofs(SaveDataDirectory / path, std::ios::binary);
			if (!ofs) return false;

			ofs.write(binary.data(), static_cast<std::streamsize>(binary.size()));
			return ofs.good();
		}

		/// <summary>
		/// バイナリデータをロードする
		/// </summary>
		/// <param name="path">ロード元 : セーブデータディレクトリからの相対パス</param>
		/// <param name="outBinary">ロードしたバイナリデータ
		/// <returns>成功したら true, 失敗したら false</returns>
		static bool Load(const std::filesystem::path& path, std::string& outBinary)
		{
			std::ifstream ifs(SaveDataDirectory / path, std::ios::binary);
			if (!ifs) return false;

			ifs.seekg(0, std::ios::end);
			const std::streamsize size = ifs.tellg();
			ifs.seekg(0, std::ios::beg);

			outBinary.resize(static_cast<std::size_t>(size));
			ifs.read(outBinary.data(), size);

			return ifs.good();
		}
	};
}
