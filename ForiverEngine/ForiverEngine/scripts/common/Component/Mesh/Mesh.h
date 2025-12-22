#pragma once

#include <scripts/common/Math/Include.h>
#include <scripts/headers/D3D12Defines.h>

namespace ForiverEngine
{
	struct Mesh
	{
		// 時計回りに結線する!!
		std::vector<VertexData> vertices;
		std::vector<std::uint32_t> indices;

		// [テクスチャ構造]
		// 凡例 : Up, Down, Right, Left, Forward, Backward
		// 1枚のテクスチャに2つ詰め込んでいるが、読み取りを切り替えるのはシェーダー側で行うので、下半分は無いものとしてUV値を設定する
		// [L][R][B][F]
		// [U][D][ ][ ]
		// [L][R][B][F]
		// [U][D][ ][ ]

		static Mesh CreateCube(std::uint32_t textureIndex)
		{
			return
			{
				// 頂点順 : 左下, 左上, 右下, 右上
				.vertices =
				{
					// Up
					{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.00f, 0.50f), Vector3::Up()      , textureIndex },
					{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(0.00f, 0.25f), Vector3::Up()      , textureIndex },
					{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.50f), Vector3::Up()      , textureIndex },
					{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.25f, 0.25f), Vector3::Up()      , textureIndex },

					// Down
					{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(0.25f, 0.50f), Vector3::Down()    , textureIndex },
					{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.25f, 0.25f), Vector3::Down()    , textureIndex },
					{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.50f, 0.50f), Vector3::Down()    , textureIndex },
					{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.50f, 0.25f), Vector3::Down()    , textureIndex },

					// Right
					{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.25f, 0.25f), Vector3::Right()   , textureIndex },
					{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.00f), Vector3::Right()   , textureIndex },
					{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.50f, 0.25f), Vector3::Right()   , textureIndex },
					{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.50f, 0.00f), Vector3::Right()   , textureIndex },

					// Left
					{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(0.00f, 0.25f), Vector3::Left()    , textureIndex },
					{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(0.00f, 0.00f), Vector3::Left()    , textureIndex },
					{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.25f, 0.25f), Vector3::Left()    , textureIndex },
					{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.25f, 0.00f), Vector3::Left()    , textureIndex },

					// Forward
					{ Vector4(+0.5f, -0.5f, +0.5f), Vector2(0.75f, 0.25f), Vector3::Forward() , textureIndex },
					{ Vector4(+0.5f, +0.5f, +0.5f), Vector2(0.75f, 0.00f), Vector3::Forward() , textureIndex },
					{ Vector4(-0.5f, -0.5f, +0.5f), Vector2(1.00f, 0.25f), Vector3::Forward() , textureIndex },
					{ Vector4(-0.5f, +0.5f, +0.5f), Vector2(1.00f, 0.00f), Vector3::Forward() , textureIndex },

					// Backward
					{ Vector4(-0.5f, -0.5f, -0.5f), Vector2(0.50f, 0.25f), Vector3::Backward(), textureIndex },
					{ Vector4(-0.5f, +0.5f, -0.5f), Vector2(0.50f, 0.00f), Vector3::Backward(), textureIndex },
					{ Vector4(+0.5f, -0.5f, -0.5f), Vector2(0.75f, 0.25f), Vector3::Backward(), textureIndex },
					{ Vector4(+0.5f, +0.5f, -0.5f), Vector2(0.75f, 0.00f), Vector3::Backward(), textureIndex },
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

		// インデックスは y,z,x の順にアクセスする
		// 各要素の値はブロックの識別値で、現在はテクスチャインデックスと同じにする
		static Mesh CreateFromTerrainData(const std::vector<std::vector<std::vector<std::uint32_t>>>& terrainData)
		{
			struct Local
			{
				// ブロックのフェースが遮られているかチェックする
				static bool IsBlockFaceOccluded(
					const std::vector<std::vector<std::vector<std::uint32_t>>>& terrain,
					Lattice3 position, Lattice3 faceNormal
				)
				{
					// フェースに隣接するブロック
					// ここにブロックがあるかないかで、面が遮られているか判定する
					const Lattice3 checkPosition = position + faceNormal;

					// データの範囲外
					if (checkPosition.x < 0 || checkPosition.x >= static_cast<int>(terrain[0][0].size())
						|| checkPosition.y < 0 || checkPosition.y >= static_cast<int>(terrain.size())
						|| checkPosition.z < 0 || checkPosition.z >= static_cast<int>(terrain[0].size()))
					{
						return false;
					}

					// ブロックが無い
					if (terrain[checkPosition.y][checkPosition.z][checkPosition.x] == 0)
					{
						return false;
					}

					return true;
				}

				// 指定されたフェースをメッシュに追加する
				// ワールドから見た向きで、テクスチャの配置は固定する
				static void AddFace(
					Mesh& mesh,
					Lattice3 position, Lattice3 faceNormal, std::uint32_t textureIndex
				)
				{
					// 無効な法線ならダメ
					if (std::abs(faceNormal.x) + std::abs(faceNormal.y) + std::abs(faceNormal.z) != 1) return;

					// [indexBegin, indexBegin+3] が今回追加した分のインデックス
					const std::uint16_t indexBegin = static_cast<std::uint16_t>(mesh.vertices.size());
					mesh.indices.push_back(indexBegin + 0);
					mesh.indices.push_back(indexBegin + 1);
					mesh.indices.push_back(indexBegin + 2);
					mesh.indices.push_back(indexBegin + 2);
					mesh.indices.push_back(indexBegin + 1);
					mesh.indices.push_back(indexBegin + 3);

					const Vector3 positionAsVec = Vector3(
						static_cast<float>(position.x),
						static_cast<float>(position.y),
						static_cast<float>(position.z)
					);
					if (faceNormal == Lattice3::Up())
					{
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, +0.5f, -0.5f)), Vector2(0.00f, 0.50f), Vector3::Up(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, +0.5f, +0.5f)), Vector2(0.00f, 0.25f), Vector3::Up(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, +0.5f, -0.5f)), Vector2(0.25f, 0.50f), Vector3::Up(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, +0.5f, +0.5f)), Vector2(0.25f, 0.25f), Vector3::Up(), textureIndex);
					}
					else if (faceNormal == Lattice3::Down())
					{
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, -0.5f, +0.5f)), Vector2(0.25f, 0.50f), Vector3::Down(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, -0.5f, -0.5f)), Vector2(0.25f, 0.25f), Vector3::Down(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, -0.5f, +0.5f)), Vector2(0.50f, 0.50f), Vector3::Down(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, -0.5f, -0.5f)), Vector2(0.50f, 0.25f), Vector3::Down(), textureIndex);
					}
					else if (faceNormal == Lattice3::Right())
					{
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, -0.5f, -0.5f)), Vector2(0.25f, 0.25f), Vector3::Right(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, +0.5f, -0.5f)), Vector2(0.25f, 0.00f), Vector3::Right(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, -0.5f, +0.5f)), Vector2(0.50f, 0.25f), Vector3::Right(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, +0.5f, +0.5f)), Vector2(0.50f, 0.00f), Vector3::Right(), textureIndex);
					}
					else if (faceNormal == Lattice3::Left())
					{
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, -0.5f, +0.5f)), Vector2(0.00f, 0.25f), Vector3::Left(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, +0.5f, +0.5f)), Vector2(0.00f, 0.00f), Vector3::Left(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, -0.5f, -0.5f)), Vector2(0.25f, 0.25f), Vector3::Left(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, +0.5f, -0.5f)), Vector2(0.25f, 0.00f), Vector3::Left(), textureIndex);
					}
					else if (faceNormal == Lattice3::Forward())
					{
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, -0.5f, +0.5f)), Vector2(0.75f, 0.25f), Vector3::Forward(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, +0.5f, +0.5f)), Vector2(0.75f, 0.00f), Vector3::Forward(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, -0.5f, +0.5f)), Vector2(1.00f, 0.25f), Vector3::Forward(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, +0.5f, +0.5f)), Vector2(1.00f, 0.00f), Vector3::Forward(), textureIndex);
					}
					else // faceNormal == Lattice3::Backward()
					{
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, -0.5f, -0.5f)), Vector2(0.50f, 0.25f), Vector3::Backward(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(-0.5f, +0.5f, -0.5f)), Vector2(0.50f, 0.00f), Vector3::Backward(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, -0.5f, -0.5f)), Vector2(0.75f, 0.25f), Vector3::Backward(), textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionAsVec + Vector3(+0.5f, +0.5f, -0.5f)), Vector2(0.75f, 0.00f), Vector3::Backward(), textureIndex);
					}
				}
			};



			Mesh mesh = { .vertices = {}, .indices = {} };
			// ある程度 reserve しておく
			mesh.vertices.reserve(4096);
			mesh.indices.reserve(1024);

			for (int y = 0; y < static_cast<int>(terrainData.size()); ++y)
				for (int z = 0; z < static_cast<int>(terrainData[0].size()); ++z)
					for (int x = 0; x < static_cast<int>(terrainData[0][0].size()); ++x)
					{
						const std::uint32_t block = terrainData[y][z][x];
						if (block == 0) continue; // ブロックが無いならスキップ

						// ブロックの座標 (格子点なので、配列のインデックスと同義)
						const Lattice3 position = Lattice3(x, y, z);

						// 面一覧 (具体的には、面の法線ベクトル)
						constexpr Lattice3 faceNormals[] =
						{
							Lattice3::Up(),
							Lattice3::Down(),
							Lattice3::Right(),
							Lattice3::Left(),
							Lattice3::Forward(),
							Lattice3::Backward(),
						};

						for (const auto& faceNormal : faceNormals)
						{
							if (!Local::IsBlockFaceOccluded(terrainData, position, faceNormal))
							{
								Local::AddFace(mesh, position, faceNormal, block);
							}
						}
					}

			// 頂点が1つも無い場合、ダミーで何か入れておく
			if (mesh.vertices.size() <= 0)
			{
				mesh = CreateCube(0); // 空気なので、表示はされない
			}

			return mesh;
		}
	};
}
