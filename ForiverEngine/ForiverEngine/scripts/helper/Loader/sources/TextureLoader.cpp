#include "../headers/TextureLoader.h"

#include <DirectXTex.h>

#if _DEBUG
#pragma comment(lib, "DirectXTex_x64_Debug.lib")
#else
#pragma comment(lib, "DirectXTex_x64_Release.lib")
#endif

namespace ForiverEngine
{
	Texture TextureLoader::Load(const std::string& path)
	{
		DirectX::TexMetadata metadata = {};
		DirectX::ScratchImage scratchImage = {};

		if (DirectX::LoadFromWICFile(
			StringUtils::UTF8ToUTF16(path).c_str(),
			DirectX::WIC_FLAGS_NONE, // 特別なことはしない
			&metadata, // メタデータはここに入る
			scratchImage // 実際のデータはここに入る
		) != S_OK)
		{
			return Texture();
		}

		// メタデータの値が適切かチェック
		// 列挙型の整数値が同じなので、static_cast で変換できる
		if (static_cast<GraphicsBufferType>(metadata.dimension) != GraphicsBufferType::Texture2D) return Texture();
		if (!IsValidFormat(static_cast<Format>(metadata.format))) return Texture();
		if (metadata.arraySize != 1) return Texture();
		if (metadata.mipLevels != 1) return Texture();

		const DirectX::Image* image = scratchImage.GetImage(
			0, // ミップは使わない
			0, // テクスチャ配列のインデックス (0でOK)
			0  // 3Dテクスチャのスライスインデックス (0でOK)
		);

		// 生データをコピーして、外部スコープに返せるようにする
		std::vector<std::uint8_t> rawData;
		rawData.resize(image->slicePitch);
		std::memcpy(rawData.data(), image->pixels, image->slicePitch);

		Texture texture = Texture(
			std::move(rawData),
			Lattice3(static_cast<int>(metadata.width), static_cast<int>(metadata.height), 1),
			static_cast<Format>(metadata.format)
		);

		// バイトサイズが合っているかをチェック
		if (static_cast<int>(image->rowPitch) != texture.GetRowBytes()) return Texture();
		if (static_cast<int>(image->slicePitch) != texture.GetSliceBytes()) return Texture();

		return texture;
	}

	Texture TextureLoader::LoadAsArray(const std::vector<std::string>& paths)
	{
		if (paths.empty()) return Texture();

		// 1枚目で基準を決め、2枚目以降はそれと同じかチェックする
		// 各ロードされたテクスチャは、sliceCount == 1 である必要がある
		// 生データを結合して格納する
		Texture texture = Texture();
		texture.size.z = static_cast<int>(paths.size());

		for (int i = 0; i < texture.size.z; ++i)
		{
			const Texture loadedTexture = Load(paths[i]);
			if (!loadedTexture.IsValid()) return Texture();

			// 1枚目で基準を決める
			if (i == 0)
			{
				texture.size.x = loadedTexture.size.x;
				texture.size.y = loadedTexture.size.y;
				texture.format = loadedTexture.format;

				if (loadedTexture.size.z != 1) return Texture();
			}
			// 2枚目以降は基準と同じかチェック
			else
			{
				if (loadedTexture.size.x != texture.size.x) return Texture();
				if (loadedTexture.size.y != texture.size.y) return Texture();
				if (loadedTexture.format != texture.format) return Texture();

				if (loadedTexture.size.z != 1) return Texture();
			}

			// 生データを結合
			texture.data.insert(
				texture.data.end(),
				loadedTexture.data.begin(),
				loadedTexture.data.end()
			);
		}

		return texture;
	}
}
