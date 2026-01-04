#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include "./IMesh.h"

namespace ForiverEngine
{
	struct Mesh : public IMesh<VertexData>
	{
		// 時計回りに結線する!!
		std::vector<VertexData> vertices{};
		std::vector<std::uint32_t> indices{};

		const std::vector<VertexData>& GetVertices() const override
		{
			return vertices;
		}
		const std::vector<std::uint32_t>& GetIndices() const override
		{
			return indices;
		}

		// [テクスチャ構造]
		// 凡例 : Up, Down, Right, Left, Forward, Backward
		// 1枚のテクスチャに2つ詰め込んでいるが、読み取りを切り替えるのはシェーダー側で行うので、下半分は無いものとしてUV値を設定する
		// [L][R][B][F]
		// [U][D][ ][ ]
		// [L][R][B][F]
		// [U][D][ ][ ]

		static Mesh CreateCube(const Vector3& centerWorldPosition, std::uint32_t textureIndex)
		{
			Mesh mesh = {};

			// 頂点順 : 左下, 左上, 右下, 右上
			mesh.vertices =
			{
				// Up
				{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.00f, 0.50f), Vector3::Up()      , centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(0.00f, 0.25f), Vector3::Up()      , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.50f), Vector3::Up()      , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.25f, 0.25f), Vector3::Up()      , centerWorldPosition, textureIndex },

				// Down
				{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(0.25f, 0.50f), Vector3::Down()    , centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.25f, 0.25f), Vector3::Down()    , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.50f, 0.50f), Vector3::Down()    , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.50f, 0.25f), Vector3::Down()    , centerWorldPosition, textureIndex },

				// Right
				{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.25f, 0.25f), Vector3::Right()   , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.00f), Vector3::Right()   , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.50f, 0.25f), Vector3::Right()   , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.50f, 0.00f), Vector3::Right()   , centerWorldPosition, textureIndex },

				// Left
				{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(0.00f, 0.25f), Vector3::Left()    , centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(0.00f, 0.00f), Vector3::Left()    , centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.25f, 0.25f), Vector3::Left()    , centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.00f), Vector3::Left()    , centerWorldPosition, textureIndex },

				// Forward
				{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.75f, 0.25f), Vector3::Forward() , centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.75f, 0.00f), Vector3::Forward() , centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(1.00f, 0.25f), Vector3::Forward() , centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(1.00f, 0.00f), Vector3::Forward() , centerWorldPosition, textureIndex },

				// Backward
				{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.50f, 0.25f), Vector3::Backward(), centerWorldPosition, textureIndex },
				{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.50f, 0.00f), Vector3::Backward(), centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.75f, 0.25f), Vector3::Backward(), centerWorldPosition, textureIndex },
				{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.75f, 0.00f), Vector3::Backward(), centerWorldPosition, textureIndex },
			};
			mesh.indices =
			{
				 0,  1,  2,  2,  1,  3, // Up
				 4,  5,  6,  6,  5,  7, // Down
				 8,  9, 10, 10,  9, 11, // Right
				12, 13, 14, 14, 13, 15, // Left
				16, 17, 18, 18, 17, 19, // Forward
				20, 21, 22, 22, 21, 23, // Backward
			};

			return mesh;
		}
	};
}
