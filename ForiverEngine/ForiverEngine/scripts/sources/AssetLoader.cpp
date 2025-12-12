#include "../headers/AssetLoader.h"

#include <DirectXTex.h>

#if _DEBUG
#pragma comment(lib, "DirectXTex_x64_Debug.lib")
#else
#pragma comment(lib, "DirectXTex_x64_Release.lib")
#endif

namespace ForiverEngine
{
	Texture AssetLoader::LoadTexture(const std::string& path)
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
			return Texture{};
		}

		const DirectX::Image* image = scratchImage.GetImage(
			0, // ミップは使わない
			0, // テクスチャ配列のインデックス (0でOK)
			0 // 3Dテクスチャのスライスインデックス (0でOK)
		);

		// 生データをコピーして、外部スコープに返せるようにする
		std::vector<std::uint8_t> rawData;
		rawData.resize(image->slicePitch);
		std::memcpy(rawData.data(), image->pixels, image->slicePitch);

		return Texture
		{
			.data = std::move(rawData),
			.textureType = static_cast<GraphicsBufferType>(metadata.dimension),
			.format = static_cast<Format>(metadata.format),
			.width = static_cast<int>(metadata.width),
			.height = static_cast<int>(metadata.height),
			.rowSize = static_cast<int>(image->rowPitch),
			.sliceSize = static_cast<int>(image->slicePitch),
			.sliceCount = static_cast<int>(metadata.arraySize),
			.mipLevels = static_cast<int>(metadata.mipLevels),
		};
	}
}
