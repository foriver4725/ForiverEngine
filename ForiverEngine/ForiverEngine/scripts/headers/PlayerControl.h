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

		static Lattice2 GetChunkIndexAtPosition(const Vector3& position)
		{
			const int chunkX = static_cast<int>(std::floor(position.x / Terrain::ChunkSize));
			const int chunkZ = static_cast<int>(std::floor(position.z / Terrain::ChunkSize));
			return Lattice2(chunkX, chunkZ);
		}

		static void Rotate(Transform& transform, const Vector2& rotateInput, const Vector2& rotateSpeed, float deltaSeconds)
		{
			const Quaternion rotationAmount =
				Quaternion::FromAxisAngle(Vector3::Up(), rotateInput.x * rotateSpeed.x * deltaSeconds) *
				Quaternion::FromAxisAngle(transform.GetRight(), -rotateInput.y * rotateSpeed.y * deltaSeconds);

			const Quaternion newRotation = rotationAmount * transform.rotation;

			if (std::abs((newRotation * Vector3::Forward()).y) < 0.99f) // 上下回転を制限 (前方向ベクトルのy成分で判定)
				transform.rotation = newRotation;
		}

		static void MoveH(Transform& transform, const Vector2& moveInput, float moveSpeed, float deltaSeconds)
		{
			Vector3 moveDirection = transform.rotation * Vector3(moveInput.x, 0.0f, moveInput.y);
			moveDirection.y = 0.0f; // 水平成分のみ
			moveDirection.Norm(); // 最後に正規化する

			transform.position += moveDirection * (moveSpeed * deltaSeconds);
		}

		// 足元より下である中で、最も高いブロックのY座標を取得する (無いなら多分、チャンクの高さの最小値-1を返す)
		template<std::uint32_t ChunkSize>
		static int GetFloorHeight(
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

			int y = std::numeric_limits<int>::min();
			for (int z = minZ; z <= maxZ; ++z)
				for (int x = minX; x <= maxX; ++x)
				{
					const int height = terrain.GetFloorHeight(x, z, maxY);
					y = std::max(y, height);
				}

			return y;
		}

		// 視線の高さより上である中で、最も低いブロックのY座標を取得する (無いなら多分、チャンクの高さの最大値+1を返す)
		template<std::uint32_t ChunkSize>
		static int GetCeilHeight(
			const std::array<std::array<Terrain, ChunkSize>, ChunkSize>& terrainChunks,
			const Vector3& position, // 視線の座標 (地面の高さはこれ以下になることはない)
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
			const int minY = std::clamp(static_cast<int>(std::ceil(localPosition.y)), 0, Terrain::ChunkHeight - 1);
			const int minZ = std::clamp(static_cast<int>(std::round(localPosition.z - size.z * 0.5f)), 0, Terrain::ChunkSize - 1);
			const int maxZ = std::clamp(static_cast<int>(std::round(localPosition.z + size.z * 0.5f)), 0, Terrain::ChunkSize - 1);

			int y = std::numeric_limits<int>::max();
			for (int z = minZ; z <= maxZ; ++z)
				for (int x = minX; x <= maxX; ++x)
				{
					const int height = terrain.GetCeilHeight(x, z, minY);
					y = std::min(y, height);
				}

			return y;
		}

		template<std::uint32_t ChunkSize>
		static bool IsOverlappingWithTerrain(
			const std::array<std::array<Terrain, ChunkSize>, ChunkSize>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
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

			const int minX = std::clamp(static_cast<int>(std::round(localPosition.x - size.x * 0.5f)), 0, Terrain::ChunkSize - 1);
			const int maxX = std::clamp(static_cast<int>(std::round(localPosition.x + size.x * 0.5f)), 0, Terrain::ChunkSize - 1);
			const int minY = std::clamp(static_cast<int>(std::round(localPosition.y)), 0, Terrain::ChunkHeight - 1);
			const int maxY = std::clamp(static_cast<int>(std::round(localPosition.y + size.y)), 0, Terrain::ChunkHeight - 1);
			const int minZ = std::clamp(static_cast<int>(std::round(localPosition.z - size.z * 0.5f)), 0, Terrain::ChunkSize - 1);
			const int maxZ = std::clamp(static_cast<int>(std::round(localPosition.z + size.z * 0.5f)), 0, Terrain::ChunkSize - 1);

			for (int y = minY; y <= maxY; ++y)
				for (int z = minZ; z <= maxZ; ++z)
					for (int x = minX; x <= maxX; ++x)
					{
						if (terrain.GetBlock(x, y, z) != Block::Air)
							return true;
					}

			return false;
		}
	};
}
