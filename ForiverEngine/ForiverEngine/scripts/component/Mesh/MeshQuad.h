#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include "./IMesh.h"

namespace ForiverEngine
{
	struct MeshQuad : public IMesh<VertexDataQuad>
	{
		// 時計回りに結線する!!
		std::vector<VertexDataQuad> vertices{};
		std::vector<std::uint32_t> indices{};

		const std::vector<VertexDataQuad>& GetVertices() const override
		{
			return vertices;
		}
		const std::vector<std::uint32_t>& GetIndices() const override
		{
			return indices;
		}

		static MeshQuad CreateFullSized()
		{
			MeshQuad mesh = {};

			// 頂点順 : 左下, 左上, 右下, 右上
			mesh.vertices =
			{
				{ Vector4(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f) }, // 左下
				{ Vector4(-1.0f, +1.0f, 0.0f), Vector2(0.0f, 0.0f) }, // 左上
				{ Vector4(+1.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f) }, // 右下
				{ Vector4(+1.0f, +1.0f, 0.0f), Vector2(1.0f, 0.0f) }, // 右上
			};
			mesh.indices =
			{
				0, 1, 2,
				2, 1, 3,
			};

			return mesh;
		}
	};
}
