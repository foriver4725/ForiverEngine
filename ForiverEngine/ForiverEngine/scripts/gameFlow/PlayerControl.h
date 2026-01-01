#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./Terrain.h"

namespace ForiverEngine
{
	class PlayerControl final
	{
	public:
		DELETE_DEFAULT_METHODS(PlayerControl);

		// 与えられた座標がどのブロックの位置にあるかを計算し、格子座標で返す
		// X,Y,Z の1要素版
		static int GetBlockPosition(const float position)
		{
			return std::round(position);
		}

		// 与えられた座標がどのブロックの位置にあるかを計算し、格子座標で返す
		static Lattice3 GetBlockPosition(const Vector3& position)
		{
			return Lattice3(
				GetBlockPosition(position.x),
				GetBlockPosition(position.y),
				GetBlockPosition(position.z)
			);
		}

		static Vector3 GetFootPosition(const Vector3& position, float eyeHeight)
		{
			return position - Vector3::Up() * eyeHeight;
		}

		// ワールド座標 -> チャンクのインデックス
		static Lattice2 GetChunkIndex(const Vector3& position)
		{
			return Lattice2(
				std::floor(position.x / Terrain::ChunkSize),
				std::floor(position.z / Terrain::ChunkSize)
			);
		}

		// インデックスが、指定された範囲内であるか [0, count)
		static bool IsIndexInBounds(int index, int count)
		{
			return 0 <= index && index < count;
		}

		// チャンクのインデックスが、地形全体の範囲内であるか
		static bool IsChunkInBounds(const Lattice2& chunkIndex, int chunkCount)
		{
			return IsIndexInBounds(chunkIndex.x, chunkCount)
				&& IsIndexInBounds(chunkIndex.y, chunkCount);
		}

		// ワールド座標 -> チャンク内のローカル座標 ([0.0f, ChunkSize))
		static Vector3 GetChunkLocalPosition(const Vector3& position)
		{
			return Vector3(
				std::fmod(position.x, static_cast<float>(Terrain::ChunkSize)),
				position.y,
				std::fmod(position.z, static_cast<float>(Terrain::ChunkSize))
			);
		}

		// ワールド座標 -> チャンク内のローカル座標 ([0.0f, ChunkSize))
		static Lattice3 GetChunkLocalPosition(const Lattice3& position)
		{
			return Lattice3(
				position.x % Terrain::ChunkSize,
				position.y,
				position.z % Terrain::ChunkSize
			);
		}

		// コリジョン立方体が、ブロックの整数座標でどの範囲にまたがっているかを計算する
		// rangeXZ, rangeY で、戻り値の範囲を制限できる
		static std::tuple<Lattice2, Lattice2, Lattice2> CalculateCollisionBlockBoundary(
			const Vector3& minPosition, const Vector3& collisionSize, const Lattice2& rangeXZ, const Lattice2& rangeY)
		{
			const Vector3 maxPosition = minPosition + collisionSize;

			const Lattice3 blockMinPosition = GetBlockPosition(minPosition);
			const Lattice3 blockMaxPosition = GetBlockPosition(maxPosition);

			const int blockXMin = std::clamp(blockMinPosition.x, rangeXZ.x, rangeXZ.y);
			const int blockXMax = std::clamp(blockMaxPosition.x, rangeXZ.x, rangeXZ.y);
			const int blockYMin = std::clamp(blockMinPosition.y, rangeY.x, rangeY.y);
			const int blockYMax = std::clamp(blockMaxPosition.y, rangeY.x, rangeY.y);
			const int blockZMin = std::clamp(blockMinPosition.z, rangeXZ.x, rangeXZ.y);
			const int blockZMax = std::clamp(blockMaxPosition.z, rangeXZ.x, rangeXZ.y);

			const Lattice2 RangeX = Lattice2(blockXMin, blockXMax);
			const Lattice2 RangeY = Lattice2(blockYMin, blockYMax);
			const Lattice2 RangeZ = Lattice2(blockZMin, blockZMax);

			return { RangeX, RangeY, RangeZ };
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
		// TODO: 必ず1チャンクしか見ないので、当たり判定がチャンクの境界に跨っているとバグる
		template<int ChunkSize>
		static int GetFloorHeight(
			const std::array<std::array<Terrain, ChunkSize>, ChunkSize>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const Lattice2 chunkIndex = GetChunkIndex(position);
			if (!IsChunkInBounds(chunkIndex, ChunkSize))
				return -1;
			const Terrain& terrain = terrainChunks[chunkIndex.x][chunkIndex.y];

			const Vector3 localMinPosition = GetChunkLocalPosition(position) - Vector3(size.x * 0.5f, 0.0f, size.z * 0.5f);
			const auto [rangeX, rangeY, rangeZ] = CalculateCollisionBlockBoundary(localMinPosition, size,
				Lattice2(0, Terrain::ChunkSize - 1), Lattice2(0, Terrain::ChunkHeight - 1));

			int y = std::numeric_limits<int>::min();
			for (int z = rangeZ.x; z <= rangeZ.y; ++z)
				for (int x = rangeX.x; x <= rangeX.y; ++x)
				{
					const int height = terrain.GetFloorHeight(x, z, rangeY.x); // 余裕を持たせて、足元にあるブロックから調べる
					y = std::max(y, height);
				}

			return y;
		}

		// 頭上より上である中で、最も低いブロックのY座標を取得する (無いなら多分、チャンクの高さの最大値+1を返す)
		// TODO: 必ず1チャンクしか見ないので、当たり判定がチャンクの境界に跨っているとバグる
		template<int ChunkSize>
		static int GetCeilHeight(
			const std::array<std::array<Terrain, ChunkSize>, ChunkSize>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const Lattice2 chunkIndex = GetChunkIndex(position);
			if (!IsChunkInBounds(chunkIndex, ChunkSize))
				return -1;
			const Terrain& terrain = terrainChunks[chunkIndex.x][chunkIndex.y];

			const Vector3 localMinPosition = GetChunkLocalPosition(position) - Vector3(size.x * 0.5f, 0.0f, size.z * 0.5f);
			const auto [rangeX, rangeY, rangeZ] = CalculateCollisionBlockBoundary(localMinPosition, size,
				Lattice2(0, Terrain::ChunkSize - 1), Lattice2(0, Terrain::ChunkHeight - 1));

			int y = std::numeric_limits<int>::max();
			for (int z = rangeZ.x; z <= rangeZ.y; ++z)
				for (int x = rangeX.x; x <= rangeX.y; ++x)
				{
					const int height = terrain.GetCeilHeight(x, z, rangeY.y); // 余裕を持たせて、頭上にあるブロックから調べる
					y = std::min(y, height);
				}

			return y;
		}

		template<int ChunkSize>
		static bool IsOverlappingWithTerrain(
			const std::array<std::array<Terrain, ChunkSize>, ChunkSize>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const Lattice2 chunkIndex = GetChunkIndex(position);
			if (!IsChunkInBounds(chunkIndex, ChunkSize))
				return false;
			const Terrain& terrain = terrainChunks[chunkIndex.x][chunkIndex.y];

			const Vector3 localMinPosition = GetChunkLocalPosition(position) - Vector3(size.x * 0.5f, 0.0f, size.z * 0.5f);
			const Vector3 localMaxPosition = localMinPosition + size;
			const bool collisionExistsInMultipleChunkX = GetBlockPosition(localMaxPosition.x) >= Terrain::ChunkSize;
			const bool collisionExistsInMultipleChunkZ = GetBlockPosition(localMaxPosition.z) >= Terrain::ChunkSize;

			const auto [rangeX, rangeY, rangeZ] = CalculateCollisionBlockBoundary(localMinPosition, size,
				Lattice2(0, Terrain::ChunkSize - 1), Lattice2(0, Terrain::ChunkHeight - 1));

			for (int y = rangeY.x; y <= rangeY.y; ++y)
				for (int z = rangeZ.x; z <= rangeZ.y; ++z)
					for (int x = rangeX.x; x <= rangeX.y; ++x)
					{
						if (terrain.GetBlock(x, y, z) != Block::Air)
							return true;
					}

			// コリジョンがX方向にチャンクを跨いでいる
			if (collisionExistsInMultipleChunkX && IsIndexInBounds(chunkIndex.x + 1, ChunkSize))
			{
				const Terrain& terrainNextX = terrainChunks[chunkIndex.x + 1][chunkIndex.y];
				const int localBlockMaxXAtNextX = GetChunkLocalPosition(GetBlockPosition(localMaxPosition)).x;

				for (int y = rangeY.x; y <= rangeY.y; ++y)
					for (int z = rangeZ.x; z <= rangeZ.y; ++z)
						for (int x = 0; x <= localBlockMaxXAtNextX; ++x)
						{
							if (terrainNextX.GetBlock(x, y, z) != Block::Air)
								return true;
						}
			}
			// コリジョンがZ方向にチャンクを跨いでいる
			if (collisionExistsInMultipleChunkZ && IsIndexInBounds(chunkIndex.y + 1, ChunkSize))
			{
				const Terrain& terrainNextZ = terrainChunks[chunkIndex.x][chunkIndex.y + 1];
				const int localBlockMaxZAtNextZ = GetChunkLocalPosition(GetBlockPosition(localMaxPosition)).z;

				for (int y = rangeY.x; y <= rangeY.y; ++y)
					for (int z = 0; z <= localBlockMaxZAtNextZ; ++z)
						for (int x = rangeX.x; x <= rangeX.y; ++x)
						{
							if (terrainNextZ.GetBlock(x, y, z) != Block::Air)
								return true;
						}
			}
			// コリジョンがX,Z両方向にチャンクを跨いでいる
			if (collisionExistsInMultipleChunkX && collisionExistsInMultipleChunkZ
				&& IsChunkInBounds(Lattice2(chunkIndex.x + 1, chunkIndex.y + 1), ChunkSize))
			{
				const Terrain& terrainNextXZ = terrainChunks[chunkIndex.x + 1][chunkIndex.y + 1];
				const int localBlockMaxXAtNextX = GetChunkLocalPosition(GetBlockPosition(localMaxPosition)).x;
				const int localBlockMaxZAtNextZ = GetChunkLocalPosition(GetBlockPosition(localMaxPosition)).z;

				for (int y = rangeY.x; y <= rangeY.y; ++y)
					for (int z = 0; z <= localBlockMaxZAtNextZ; ++z)
						for (int x = 0; x <= localBlockMaxXAtNextX; ++x)
						{
							if (terrainNextXZ.GetBlock(x, y, z) != Block::Air)
								return true;
						}
			}

			return false;
		}
	};
}
