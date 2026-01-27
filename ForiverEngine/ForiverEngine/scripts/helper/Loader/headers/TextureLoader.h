#pragma once

#include <scripts/common/Include.h>
#include "../Texture.h"

namespace ForiverEngine
{
	class TextureLoader final
	{
	public:
		DELETE_DEFAULT_METHODS(TextureLoader);

		/// <summary>
		/// <para>テクスチャをロードして返す (失敗したら空の Texture を返す)</para>
		/// <para>PNG のみ対応</para>
		/// </summary>
		static Texture Load(const std::string& path);

		/// <summary>
		/// <para>複数テクスチャをロードし、その順のテクスチャ配列として返す (失敗したら空の Texture を返す)</para>
		/// <para>PNG のみ対応</para>
		/// </summary>
		static Texture LoadAsArray(const std::vector<std::string>& paths);
	};
}
