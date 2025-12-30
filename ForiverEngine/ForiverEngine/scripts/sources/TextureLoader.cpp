#include "../headers/TextureLoader.h"

#include <DirectXTex.h>

#if _DEBUG
#pragma comment(lib, "DirectXTex_x64_Debug.lib")
#else
#pragma comment(lib, "DirectXTex_x64_Release.lib")
#endif

namespace ForiverEngine
{
	Texture TextureLoader::LoadTexture(const std::string& path)
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

	Texture TextureLoader::LoadTextureArray(const std::vector<std::string>& paths)
	{
		if (paths.empty())
			return Texture{};

		DirectX::TexMetadata baseMetadata = {};
		std::vector<std::uint8_t> combinedData;

		Format format = Format::Unknown;
		int width = 0;
		int height = 0;
		int rowSize = 0;
		int sliceSize = 0;

		for (int i = 0; i < static_cast<int>(paths.size()); ++i)
		{
			DirectX::ScratchImage scratchImage = {};
			DirectX::TexMetadata metadata = {};

			if (DirectX::LoadFromWICFile(
				StringUtils::UTF8ToUTF16(paths[i]).c_str(),
				DirectX::WIC_FLAGS_NONE,
				&metadata,
				scratchImage
			) != S_OK)
			{
				return Texture{};
			}

			const DirectX::Image* image = scratchImage.GetImage(0, 0, 0);
			if (!image)
				return Texture{};

			// 1枚目で基準を決める
			if (i == 0)
			{
				baseMetadata = metadata;

				format = static_cast<Format>(metadata.format);
				width = static_cast<int>(metadata.width);
				height = static_cast<int>(metadata.height);
				rowSize = static_cast<int>(image->rowPitch);
				sliceSize = static_cast<int>(image->slicePitch);

				combinedData.reserve(sliceSize * paths.size());
			}
			else
			{
				// サイズチェック
				if (metadata.width != baseMetadata.width ||
					metadata.height != baseMetadata.height)
				{
					// Texture2DArray にできない
					return Texture{};
				}

				// TODO: フォーマットチェックはしない!!
			}

			// スライス分コピー
			size_t offset = combinedData.size();
			combinedData.resize(offset + sliceSize);
			std::memcpy(
				combinedData.data() + offset,
				image->pixels,
				sliceSize
			);
		}

		return Texture
		{
			.data = std::move(combinedData),
			.textureType = GraphicsBufferType::Texture2D, // テクスチャ配列も2Dテクスチャ扱いでOK
			.format = format,
			.width = width,
			.height = height,
			.rowSize = rowSize,
			.sliceSize = sliceSize,
			.sliceCount = static_cast<int>(paths.size()),
			.mipLevels = 1, // ミップマップは作成していないので、1固定
		};
	}

	Texture TextureLoader::CreateManually(const std::vector<std::uint8_t>& data, std::size_t texelValueSize, int width, int height, Format format)
	{
		return Texture
		{
			.data = data,
			.textureType = GraphicsBufferType::Texture2D,
			.format = format,
			.width = width,
			.height = height,
			.rowSize = static_cast<int>(texelValueSize * width),
			.sliceSize = static_cast<int>(texelValueSize * width * height),
			.sliceCount = 1,
			.mipLevels = 1, // ミップマップなし
		};
	}
}
