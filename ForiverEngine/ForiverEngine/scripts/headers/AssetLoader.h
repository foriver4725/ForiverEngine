#pragma once

#include <scripts/common/Include.h>

namespace ForiverEngine
{
	// ロードされたテクスチャ
	struct Texture
	{
		void* data;
		int width;
		int height;
		//Format format;
		//Dimension dimension;
		std::size_t rowSize; // 1行分のデータサイズ
		std::size_t sliceSize; // 1スライス分のデータサイズ
		std::size_t sliceCount; // スライス数
		int mipLevels;
	};

	class AssetLoader final
	{
	public:
		DELETE_DEFAULT_METHODS(AssetLoader);
	};
}
