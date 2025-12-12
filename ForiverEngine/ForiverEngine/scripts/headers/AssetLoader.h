#pragma once

#include <scripts/common/Include.h>
#include "./D3D12Defines.h"

namespace ForiverEngine
{
	class AssetLoader final
	{
	public:
		DELETE_DEFAULT_METHODS(AssetLoader);

		/// <summary>
		/// <para>テクスチャをロードする</para>
		/// BMP, PNG, JPG など、基本的なファイル形式はこれを使えばOK
		/// </summary>
		static Texture LoadTexture(const std::string& path);
	};
}
