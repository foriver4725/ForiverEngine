#pragma once

#include <scripts/common/Include.h>
#include "./D3D12Defines.h"

namespace ForiverEngine
{
	class TextureLoader final
	{
	public:
		DELETE_DEFAULT_METHODS(TextureLoader);

		/// <summary>
		/// <para>テクスチャをロードして返す (失敗したら空の Texture を返す)</para>
		/// BMP, PNG, JPG など、基本的なファイル形式はこれを使えばOK
		/// </summary>
		static Texture LoadTexture(const std::string& path);

		/// <summary>
		/// <para>複数テクスチャをロードし、その順のテクスチャ配列として返す (失敗したら空の Texture を返す)</para>
		/// BMP, PNG, JPG など、基本的なファイル形式はこれを使えばOK
		/// </summary>
		static Texture LoadTextureArray(const std::vector<std::string>& paths);
	};
}
