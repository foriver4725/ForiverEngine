#pragma once

#include <scripts/common/Include.h>
#include "./D3D12Defines.h"

namespace ForiverEngine
{
	// ロードされたテクスチャ
	struct Texture
	{
		void* data;
		//Dimension dimension;
		int width;
		int height;
		std::size_t rowSize; // 1行分のデータサイズ
		std::size_t sliceSize; // 1スライス分のデータサイズ
		std::size_t sliceCount; // スライス数
		Format format;
		int mipLevels;
	};

	class AssetLoader final
	{
	public:
		DELETE_DEFAULT_METHODS(AssetLoader);
	};
}
