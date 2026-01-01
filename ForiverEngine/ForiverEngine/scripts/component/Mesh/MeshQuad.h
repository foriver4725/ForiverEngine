#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>

namespace ForiverEngine
{
	struct MeshQuad
	{
		// 時計回りに結線する!!
		std::vector<VertexDataQuad> vertices;
		std::vector<std::uint32_t> indices;

		static MeshQuad CreateFullSized()
		{
			return
			{
				// 頂点順 : 左下, 左上, 右下, 右上
				.vertices =
				{
					{ Vector4(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f) }, // 左下
					{ Vector4(-1.0f, +1.0f, 0.0f), Vector2(0.0f, 0.0f) }, // 左上
					{ Vector4(+1.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f) }, // 右下
					{ Vector4(+1.0f, +1.0f, 0.0f), Vector2(1.0f, 0.0f) }, // 右上
				},
				.indices =
				{
					0, 1, 2,
					2, 1, 3,
				},
			};
		}
	};
}
