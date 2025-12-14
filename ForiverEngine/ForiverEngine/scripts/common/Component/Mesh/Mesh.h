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

		static Mesh CreateCube()
		{
			return
			{
				.vertices =
				{
					// Up
					{ Vector4(-1, 1, -1), Vector2(0.25f, 0.50f), Vector3(0, 1, 0)},
					{ Vector4(-1, 1, 1), Vector2(0.25f, 0.25f), Vector3(0, 1, 0)},
					{ Vector4(1, 1, -1), Vector2(0.50f, 0.50f), Vector3(0, 1, 0)},
					{ Vector4(1, 1, 1), Vector2(0.50f, 0.25f), Vector3(0, 1, 0)},

					// Down
					{ Vector4(-1, -1, 1), Vector2(1.00f, 0.25f), Vector3(0, -1, 0)},
					{ Vector4(-1, -1, -1), Vector2(1.00f, 0.50f), Vector3(0, -1, 0)},
					{ Vector4(1, -1, 1), Vector2(0.75f, 0.25f), Vector3(0, -1, 0)},
					{ Vector4(1, -1, -1), Vector2(0.75f, 0.50f), Vector3(0, -1, 0)},

					// Right
					{ Vector4(1, -1, -1), Vector2(0.75f, 0.50f), Vector3(1, 0, 0)},
					{ Vector4(1, 1, -1), Vector2(0.50f, 0.50f), Vector3(1, 0, 0)},
					{ Vector4(1, -1, 1), Vector2(0.75f, 0.25f), Vector3(1, 0, 0)},
					{ Vector4(1, 1, 1), Vector2(0.50f, 0.25f), Vector3(1, 0, 0)},

					// Left
					{ Vector4(-1, -1, 1), Vector2(0.00f, 0.25f), Vector3(-1, 0, 0)},
					{ Vector4(-1, 1, 1), Vector2(0.25f, 0.25f), Vector3(-1, 0, 0)},
					{ Vector4(-1, -1, -1), Vector2(0.00f, 0.50f), Vector3(-1, 0, 0)},
					{ Vector4(-1, 1, -1), Vector2(0.25f, 0.50f), Vector3(-1, 0, 0)},

					// Forward
					{ Vector4(1, -1, 1), Vector2(0.50f, 0.00f), Vector3(0, 0, 1)},
					{ Vector4(1, 1, 1), Vector2(0.50f, 0.25f), Vector3(0, 0, 1)},
					{ Vector4(-1, -1, 1), Vector2(0.25f, 0.00f), Vector3(0, 0, 1)},
					{ Vector4(-1, 1, 1), Vector2(0.25f, 0.25f), Vector3(0, 0, 1)},

					// Backward
					{ Vector4(-1, -1, -1), Vector2(0.25f, 0.75f), Vector3(0, 0, -1)},
					{ Vector4(-1, 1, -1), Vector2(0.25f, 0.50f), Vector3(0, 0, -1)},
					{ Vector4(1, -1, -1), Vector2(0.50f, 0.75f), Vector3(0, 0, -1)},
					{ Vector4(1, 1, -1), Vector2(0.50f, 0.50f), Vector3(0, 0, -1)},
				},

				.indices =
				{
					0, 1, 2, 2, 1, 3,       // Up
					4, 5, 6, 6, 5, 7,       // Down
					8, 9, 10, 10, 9, 11,    // Right
					12, 13, 14, 14, 13, 15, // Left
					16, 17, 18, 18, 17, 19, // Forward
					20, 21, 22, 22, 21, 23, // Backward
				},
			};
		}
	};
}
