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

		/// <summary>
		/// <para>手動でテクスチャデータを作成して返す</para>
		/// 2Dテクスチャのみ対応
		/// </summary>
		static Texture CreateManually(const std::vector<std::uint8_t>& data, int width, int height, Format format);

		/// <summary>
		/// <para>手動でテクスチャデータを作成して返す</para>
		/// <para>生データを持たず、メタデータのみを持つデータコンテナとして機能する</para>
		/// 2Dテクスチャとして作成される
		/// </summary>
		static Texture CreateManuallyAsMetadata(std::size_t texelSize, int width, int height, Format format);
	};
}
