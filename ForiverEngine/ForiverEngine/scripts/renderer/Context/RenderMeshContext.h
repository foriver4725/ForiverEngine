#pragma once

#include "scripts/renderer/IncludeInternal.h"

namespace ForiverEngine
{
	/// <summary>
	/// 何をレンダリングするか (各リストの順番は一致させること!)
	/// </summary>
	// TODO: "Mesh" という命名は良くないか?
	struct RenderMeshContext
	{
		std::vector<VertexBufferView> vbvList;
		std::vector<IndexBufferView> ibvList;
		std::vector<int> indexCountList;
	};
}
