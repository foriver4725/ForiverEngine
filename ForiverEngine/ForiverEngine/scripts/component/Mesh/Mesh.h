#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>

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

		static Mesh CreateCube(const Vector3& centerWorldPosition, std::uint32_t textureIndex)
		{
			return
			{
				// 頂点順 : 左下, 左上, 右下, 右上
				.vertices =
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

		// x,y,z の順でアクセスする
		// 各要素の値はブロックの識別値で、現在はテクスチャインデックスと同じにする
		static Mesh CreateFromChunkData(
			const HeapMultiDimAllocator::Array3D<std::uint32_t>& chunkData, const Lattice3& chunkSize, const Lattice2& chunkIndex)
		{
			struct Local
			{
				// ブロックのフェースが遮られているかチェックする
				// ローカル座標
				static bool IsBlockFaceOccluded(
					const HeapMultiDimAllocator::Array3D<std::uint32_t>& chunkData, const Lattice3& chunkSize,
					const Lattice3& position, const Lattice3& faceNormal
				)
				{
					// フェースに隣接するブロック
					// ここにブロックがあるかないかで、面が遮られているか判定する
					const Lattice3 checkPosition = position + faceNormal;

					// データの範囲外
					if (checkPosition.x < 0 || chunkSize.x <= checkPosition.x ||
						checkPosition.y < 0 || chunkSize.y <= checkPosition.y ||
						checkPosition.z < 0 || chunkSize.z <= checkPosition.z)
					{
						return false;
					}

					// ブロックが無い
					if (chunkData[checkPosition.x][checkPosition.y][checkPosition.z] == 0)
					{
						return false;
					}

					return true;
				}

				// 指定されたフェースをメッシュに追加する
				// ワールドから見た向きで、テクスチャの配置は固定する
				// ワールド座標
				static void AddFace(
					Mesh& mesh,
					const Lattice3& position, const Lattice3& faceNormal, std::uint32_t textureIndex
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

					const Vector3 positionVec = Vector3(position);
					if (faceNormal == Lattice3::Up())
					{
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, +0.5f, -0.5f)), Vector2(0.00f, 0.50f), Vector3::Up(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, +0.5f, +0.5f)), Vector2(0.00f, 0.25f), Vector3::Up(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, +0.5f, -0.5f)), Vector2(0.25f, 0.50f), Vector3::Up(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, +0.5f, +0.5f)), Vector2(0.25f, 0.25f), Vector3::Up(),
							positionVec, textureIndex);
					}
					else if (faceNormal == Lattice3::Down())
					{
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, -0.5f, +0.5f)), Vector2(0.25f, 0.50f), Vector3::Down(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, -0.5f, -0.5f)), Vector2(0.25f, 0.25f), Vector3::Down(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, -0.5f, +0.5f)), Vector2(0.50f, 0.50f), Vector3::Down(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, -0.5f, -0.5f)), Vector2(0.50f, 0.25f), Vector3::Down(),
							positionVec, textureIndex);
					}
					else if (faceNormal == Lattice3::Right())
					{
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, -0.5f, -0.5f)), Vector2(0.25f, 0.25f), Vector3::Right(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, +0.5f, -0.5f)), Vector2(0.25f, 0.00f), Vector3::Right(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, -0.5f, +0.5f)), Vector2(0.50f, 0.25f), Vector3::Right(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, +0.5f, +0.5f)), Vector2(0.50f, 0.00f), Vector3::Right(),
							positionVec, textureIndex);
					}
					else if (faceNormal == Lattice3::Left())
					{
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, -0.5f, +0.5f)), Vector2(0.00f, 0.25f), Vector3::Left(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, +0.5f, +0.5f)), Vector2(0.00f, 0.00f), Vector3::Left(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, -0.5f, -0.5f)), Vector2(0.25f, 0.25f), Vector3::Left(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, +0.5f, -0.5f)), Vector2(0.25f, 0.00f), Vector3::Left(),
							positionVec, textureIndex);
					}
					else if (faceNormal == Lattice3::Forward())
					{
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, -0.5f, +0.5f)), Vector2(0.75f, 0.25f), Vector3::Forward(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, +0.5f, +0.5f)), Vector2(0.75f, 0.00f), Vector3::Forward(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, -0.5f, +0.5f)), Vector2(1.00f, 0.25f), Vector3::Forward(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, +0.5f, +0.5f)), Vector2(1.00f, 0.00f), Vector3::Forward(),
							positionVec, textureIndex);
					}
					else // faceNormal == Lattice3::Backward()
					{
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, -0.5f, -0.5f)), Vector2(0.50f, 0.25f), Vector3::Backward(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(-0.5f, +0.5f, -0.5f)), Vector2(0.50f, 0.00f), Vector3::Backward(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, -0.5f, -0.5f)), Vector2(0.75f, 0.25f), Vector3::Backward(),
							positionVec, textureIndex);
						mesh.vertices.emplace_back(
							Vector4(positionVec + Vector3(+0.5f, +0.5f, -0.5f)), Vector2(0.75f, 0.00f), Vector3::Backward(),
							positionVec, textureIndex);
					}
				}
			};



			Mesh mesh = { .vertices = {}, .indices = {} };
			// ある程度 reserve しておく
			mesh.vertices.reserve(4096);
			mesh.indices.reserve(1024);

			for (int xi = 0; xi < chunkSize.x; ++xi)
				for (int yi = 0; yi < chunkSize.y; ++yi)
					for (int zi = 0; zi < chunkSize.z; ++zi)
					{
						const std::uint32_t block = chunkData[xi][yi][zi];
						if (block == 0) continue; // ブロックが無いならスキップ

						// ブロックの座標 (格子点なので、配列のインデックスと同義)
						const Lattice3 localPosition = Lattice3(xi, yi, zi);
						const Lattice3 worldPosition = localPosition + Lattice3(chunkIndex.x * chunkSize.x, 0, chunkIndex.y * chunkSize.z);

						// 面一覧 (具体的には、面の法線ベクトル)
						constexpr Lattice3 FaceNormals[] =
						{
							Lattice3::Up(),
							Lattice3::Down(),
							Lattice3::Right(),
							Lattice3::Left(),
							Lattice3::Forward(),
							Lattice3::Backward(),
						};

						for (const auto& faceNormal : FaceNormals)
						{
							if (!Local::IsBlockFaceOccluded(chunkData, chunkSize, localPosition, faceNormal))
							{
								Local::AddFace(mesh, worldPosition, faceNormal, block);
							}
						}
					}

			// 頂点が1つも無い場合、ダミーで何か入れておく
			if (mesh.vertices.size() <= 0)
			{
				mesh = CreateCube(Vector3::Zero(), 0); // 空気なので、表示はされない
			}

			return mesh;
		}
	};
}
