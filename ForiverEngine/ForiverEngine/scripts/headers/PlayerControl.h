#pragma once

#include <scripts/common/Include.h>
#include "./Terrain.h"

namespace ForiverEngine
{
	class PlayerControl final
	{
	public:
		DELETE_DEFAULT_METHODS(PlayerControl);

		static Vector3 GetFootPosition(const Vector3& position, float eyeHeight)
		{
			return position - Vector3::Up() * eyeHeight;
		}

		// 与えられた座標が属する格子座標を取得する
		// 要するに、どのブロックの場所か
		static Lattice3 GetPositionAsLattice(const Vector3& position)
		{
			return Lattice3(
				static_cast<int>(std::round(position.x)),
				static_cast<int>(std::round(position.y)),
				static_cast<int>(std::round(position.z))
			);
		}

		static Lattice2 GetChunkIndexAtPosition(const Vector3& position)
		{
			const int chunkX = static_cast<int>(std::floor(position.x / Terrain::ChunkSize));
			const int chunkZ = static_cast<int>(std::floor(position.z / Terrain::ChunkSize));
			return Lattice2(chunkX, chunkZ);
		}

		static Quaternion GetRotateAmount(const Transform& transform, const Vector2& rotateInput, const Vector2& rotateSpeed, float deltaSeconds)
		{
			const Quaternion rotateAmount =
				Quaternion::FromAxisAngle(Vector3::Up(), rotateInput.x * rotateSpeed.x * deltaSeconds) *
				Quaternion::FromAxisAngle(transform.GetRight(), -rotateInput.y * rotateSpeed.y * deltaSeconds);

			const Quaternion newRotation = rotateAmount * transform.rotation;
			if (std::abs((newRotation * Vector3::Forward()).y) < 0.99f) // 上下回転を制限 (前方向ベクトルのy成分で判定)
				return rotateAmount;

			return Quaternion::Identity();
		}

		static Vector3 GetMoveHAmount(const Transform& transform, const Vector2& moveInput, float moveSpeed, float deltaSeconds)
		{
			Vector3 moveDirection = transform.rotation * Vector3(moveInput.x, 0.0f, moveInput.y);
			moveDirection.y = 0.0f; // 水平成分のみ
			moveDirection.Norm(); // 最後に正規化する

			const Vector3 moveHAmount = moveDirection * (moveSpeed * deltaSeconds);
			return moveHAmount;
		}

		// 足元より下である中で、最も高い地面の高さを取得する
		template<std::uint32_t ChunkSize>
		static int GetFootSurfaceHeight(
			const std::array<std::array<Terrain, ChunkSize>, ChunkSize>& terrainChunks,
			const Vector3& position, // 足元の座標 (地面の高さはこれより大きくなることはない)
			const Vector3& size // コリジョンのサイズ
		)
		{
			const Lattice2 chunkIndex = GetChunkIndexAtPosition(position);
			if (chunkIndex.x < 0 || chunkIndex.x >= static_cast<int>(ChunkSize)
				|| chunkIndex.y < 0 || chunkIndex.y >= static_cast<int>(ChunkSize))
			{
				return -1; // チャンク外
			}
			const Terrain& terrain = terrainChunks[chunkIndex.x][chunkIndex.y];
			const Vector3 localPosition = position - Vector3(
				static_cast<float>(chunkIndex.x * Terrain::ChunkSize),
				0.0f,
				static_cast<float>(chunkIndex.y * Terrain::ChunkSize)
			);

			const int minX = std::clamp(static_cast<int>(std::round(localPosition.x - size.x * 0.5f)), 0, Terrain::ChunkSize - 1);
			const int maxX = std::clamp(static_cast<int>(std::round(localPosition.x + size.x * 0.5f)), 0, Terrain::ChunkSize - 1);
			const int maxY = std::clamp(static_cast<int>(std::floor(localPosition.y)), 0, Terrain::ChunkHeight - 1);
			const int minZ = std::clamp(static_cast<int>(std::round(localPosition.z - size.z * 0.5f)), 0, Terrain::ChunkSize - 1);
			const int maxZ = std::clamp(static_cast<int>(std::round(localPosition.z + size.z * 0.5f)), 0, Terrain::ChunkSize - 1);

			int surfaceY = -1;
			for (int z = minZ; z <= maxZ; ++z)
				for (int x = minX; x <= maxX; ++x)
				{
					const int height = terrain.GetSurfaceHeight(x, z, maxY);
					surfaceY = std::max(surfaceY, height);
				}

			return surfaceY;
		}

		// 地形との AABB (XZ のみで大小判定. Y は足元の座標をブロックの格子座標に変換したもので固定)
		template<std::uint32_t ChunkSize>
		static bool IsOverlappingWithTerrainXZ(
			const std::array<std::array<Terrain, ChunkSize>, ChunkSize>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size, // コリジョンのサイズ
			std::vector<std::tuple<Lattice3, Vector3>>& outCollisionInfo // (衝突したブロックの位置(ワールド座標), めり込んだ量(正. Yは必ず0)) 一覧
		)
		{
			const Lattice2 chunkIndex = GetChunkIndexAtPosition(position);
			if (chunkIndex.x < 0 || chunkIndex.x >= static_cast<int>(ChunkSize)
				|| chunkIndex.y < 0 || chunkIndex.y >= static_cast<int>(ChunkSize))
			{
				return false; // チャンク外
			}
			const Terrain& terrain = terrainChunks[chunkIndex.x][chunkIndex.y];
			const Vector3 localPosition = position - Vector3(
				static_cast<float>(chunkIndex.x * Terrain::ChunkSize),
				0.0f,
				static_cast<float>(chunkIndex.y * Terrain::ChunkSize)
			);

			// プレイヤーの AABB
			const float minX = localPosition.x - size.x * 0.5f;
			const float maxX = localPosition.x + size.x * 0.5f;
			const float y = localPosition.y;
			const float minZ = localPosition.z - size.z * 0.5f;
			const float maxZ = localPosition.z + size.z * 0.5f;
			const Lattice3 minLattice = GetPositionAsLattice(Vector3(minX, y, minZ));
			const Lattice3 maxLattice = GetPositionAsLattice(Vector3(maxX, y, maxZ));
			const int yLattice = minLattice.y; // どっちでも良い

			// 衝突判定本体

			// 適当にメモリ確保しておく
			outCollisionInfo.clear();
			outCollisionInfo.reserve(4);

			bool isOverlapping = false;
			for (int z = std::max(minLattice.z, 0); z <= std::min(maxLattice.z, Terrain::ChunkSize - 1); ++z)
				for (int x = std::max(minLattice.x, 0); x <= std::min(maxLattice.x, Terrain::ChunkSize - 1); ++x)
				{
					if (terrain.GetBlock(x, yLattice, z) != Block::Air)
					{
						// ブロックの AABB
						// [x-0.5f, x+0.5f], [y-0.5f, y+0.5f], [z-0.5f, z+0.5f]

						const Lattice3 collisionedBlockPosition = Lattice3(
							x + chunkIndex.x * Terrain::ChunkSize, yLattice, z + chunkIndex.y * Terrain::ChunkSize);
						const Vector3 collisionedAmount = Vector3(
							std::min(maxX - (x - 0.5f), (x + 0.5f) - minX),
							0.0f,
							std::min(maxZ - (z - 0.5f), (z + 0.5f) - minZ)
						);

						// 一応チェック
						if (collisionedAmount.x <= 0.0f || collisionedAmount.z <= 0.0f)
							continue;

						outCollisionInfo.emplace_back(collisionedBlockPosition, collisionedAmount);
						isOverlapping |= true;
					}
				}

			return isOverlapping;
		}
	};
}
