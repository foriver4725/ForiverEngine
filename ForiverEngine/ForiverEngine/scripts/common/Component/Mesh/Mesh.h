#pragma once

#include <scripts/common/Math/Include.h>
#include <scripts/headers/D3D12Defines.h>

namespace ForiverEngine
{
	struct Mesh
	{
		// 時計回りに結線する!!
		std::vector<VertexData> vertices;
		std::vector<std::uint16_t> indices;

		// テクスチャ構造
		// 立方体を切り開いた展開図
		// TODO: テクスチャ画像の空きスペースが無駄!
		// Up, Down, Right, Left, Forward, Backward
		// [ ][F][ ][ ]
		// [L][U][R][D]
		// [ ][B][ ][ ]
		// [ ][ ][ ][ ]

		static Mesh CreateCube()
		{
			return
			{
				.vertices =
				{
					// Up
					{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.50f), Vector3::Up() },
					{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(0.25f, 0.25f), Vector3::Up() },
					{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.50f, 0.50f), Vector3::Up() },
					{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.50f, 0.25f), Vector3::Up() },

					// Down
					{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(1.00f, 0.25f), Vector3::Down() },
					{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(1.00f, 0.50f), Vector3::Down() },
					{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.75f, 0.25f), Vector3::Down() },
					{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.75f, 0.50f), Vector3::Down() },

					// Right
					{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.75f, 0.50f), Vector3::Right() },
					{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.50f, 0.50f), Vector3::Right() },
					{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.75f, 0.25f), Vector3::Right() },
					{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.50f, 0.25f), Vector3::Right() },

					// Left
					{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(0.00f, 0.25f), Vector3::Left() },
					{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(0.25f, 0.25f), Vector3::Left() },
					{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.00f, 0.50f), Vector3::Left() },
					{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.50f), Vector3::Left() },

					// Forward
					{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.50f, 0.00f), Vector3::Forward() },
					{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.50f, 0.25f), Vector3::Forward() },
					{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(0.25f, 0.00f), Vector3::Forward() },
					{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(0.25f, 0.25f), Vector3::Forward() },

					// Backward
					{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.25f, 0.75f), Vector3::Backward() },
					{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.50f), Vector3::Backward() },
					{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.50f, 0.75f), Vector3::Backward() },
					{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.50f, 0.50f), Vector3::Backward() },
				},

				.indices =
				{
					 0,  1,  2,  2,  1,  3, // Up
					 4,  5,  6,  6,  5,  7, // Down
					 8,  9, 10, 10,  9, 11, // Right
					12, 13, 14, 14, 13, 15, // Left
					16, 17, 18, 18, 17, 19, // Forward
					20, 21, 22, 22, 21, 23, // Backward
				},
			};
		}
	};
}
