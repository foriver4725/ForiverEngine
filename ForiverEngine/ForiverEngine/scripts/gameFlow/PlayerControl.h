#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./Chunk.h"

namespace ForiverEngine
{
	class PlayerControl final
	{
	public:
		DELETE_DEFAULT_METHODS(PlayerControl);

		// ブロック座標に変換 (整数座標)
		static int GetBlockPosition(const float position)
		{
			return static_cast<int>(std::round(position));
		}

		// ブロック座標に変換 (整数座標)
		static Lattice3 GetBlockPosition(const Vector3& position)
		{
			return Lattice3(
				GetBlockPosition(position.x),
				GetBlockPosition(position.y),
				GetBlockPosition(position.z)
			);
		}

		// どのチャンクに属しているかを取得
		static Lattice2 GetChunkIndex(const Lattice3& worldBlockPosition, int chunkSize = Chunk::Size)
		{
			return Lattice2(
				worldBlockPosition.x / chunkSize,
				worldBlockPosition.z / chunkSize
			);
		}

		// チャンク内のローカルブロック座標に変換
		static Lattice3 GetChunkLocalPosition(const Lattice3& worldBlockPosition, int chunkSize = Chunk::Size)
		{
			return Lattice3(
				worldBlockPosition.x % chunkSize,
				worldBlockPosition.y,
				worldBlockPosition.z % chunkSize
			);
		}

		// チャンクインデックスが有効であるか
		// 即ち、上下限を超えた値でないか
		static bool IsValidChunkIndex(const Lattice2& chunkIndex, int chunkCount = Chunk::Count)
		{
			return MathUtils::IsInRange(chunkIndex.x, 0, chunkCount)
				&& MathUtils::IsInRange(chunkIndex.y, 0, chunkCount);
		}

		// プレイヤーの足元の座標を計算
		static Vector3 GetFootPosition(const Vector3& worldPosition, float eyeHeight)
		{
			return worldPosition - Vector3::Up() * eyeHeight;
		}

		// プレイヤーのコリジョンで、XYZ 各値が最小である座標を計算
		static Vector3 GetCollisionMinPosition(const Vector3& footWorldPosition, const Vector3& collisionSize)
		{
			return Vector3(
				footWorldPosition.x - collisionSize.x * 0.5f,
				footWorldPosition.y,
				footWorldPosition.z - collisionSize.z * 0.5f
			);
		}

		static Quaternion Rotate(const Transform& transform, const Vector2& rotateInput, const Vector2& rotateSpeed, float deltaSeconds)
		{
			const Quaternion rotationAmount =
				Quaternion::FromAxisAngle(Vector3::Up(), rotateInput.x * rotateSpeed.x * deltaSeconds) *
				Quaternion::FromAxisAngle(transform.GetRight(), -rotateInput.y * rotateSpeed.y * deltaSeconds);

			const Quaternion newRotation = rotationAmount * transform.rotation;

			if (std::abs((newRotation * Vector3::Forward()).y) < 0.999f) // 上下回転を制限 (前方向ベクトルのy成分で判定)
				return newRotation;
			else
				return transform.rotation;
		}

		static Vector3 MoveH(const Transform& transform, const Vector2& moveInput, float moveSpeed, float deltaSeconds)
		{
			Vector3 moveDirection = transform.rotation * Vector3(moveInput.x, 0.0f, moveInput.y);
			moveDirection.y = 0.0f; // 水平成分のみ
			moveDirection.Norm(); // 最後に正規化する

			return transform.position + moveDirection * (moveSpeed * deltaSeconds);
		}

		struct CollisionBoundaryAsBlockInfoPerChunk
		{
			bool isContained; // このチャンクにコリジョン立方体が属しているか
			Lattice2 localChunkIndex; // 2x2 チャンク群内でのローカルチャンクインデックス (0~1 の範囲)
			Lattice2 rangeX; // チャンク内ローカルブロック座標での X方向の範囲
			Lattice2 rangeY; // チャンク内ローカルブロック座標での Y方向の範囲
			Lattice2 rangeZ; // チャンク内ローカルブロック座標での Z方向の範囲

			// 属さない 版のファクトリメソッド
			// rangeX, rangeY, rangeZ を全て (0,0) にして作成する
			static constexpr CollisionBoundaryAsBlockInfoPerChunk CreateUncontained(const Lattice2& localChunkIndex) noexcept
			{
				return CollisionBoundaryAsBlockInfoPerChunk
				{
					.isContained = false,
					.localChunkIndex = localChunkIndex,
					.rangeX = Lattice2::Zero(),
					.rangeY = Lattice2::Zero(),
					.rangeZ = Lattice2::Zero(),
				};
			}

			// 属する 版のファクトリメソッド
			static constexpr CollisionBoundaryAsBlockInfoPerChunk CreateContained(
				const Lattice2& localChunkIndex, const Lattice2& rangeX, const Lattice2& rangeY, const Lattice2& rangeZ) noexcept
			{
				return CollisionBoundaryAsBlockInfoPerChunk
				{
					.isContained = true,
					.localChunkIndex = localChunkIndex,
					.rangeX = rangeX,
					.rangeY = rangeY,
					.rangeZ = rangeZ,
				};
			}
		};

		/// <summary>
		/// <para>コリジョン立方体の範囲が、どのブロック座標に属するかを計算する</para>
		/// <para>複数チャンクに跨っている場合も考慮する</para>
		/// <para>コリジョンのxz方向のサイズは、1より小さい想定</para>
		/// <para>[最小インデックスのチャンク, X方向に1つ進んだチャンク, Z方向に1つ進んだチャンク, XZ両方向に1つ進んだチャンク] の順に情報を返す</para>
		/// <para>そのチャンクに属さない場合、XYZの範囲値は (0,0)</para>
		/// <para>[チャンクインデックスの範囲チェック]</para>
		/// <para>開始座標のチャンクで範囲外ならば、全てのチャンクで属さない扱いになる</para>
		/// <para>それ以外のチャンクで範囲外ならば、そのチャンクでのみ属さない扱いになる</para>
		/// </summary>
		static std::array<CollisionBoundaryAsBlockInfoPerChunk, 4> CalculateCollisionBoundaryAsBlock(
			const Vector3& worldPositionMin, const Vector3& collisionSize, int chunkSize = Chunk::Size, int chunkCount = Chunk::Count)
		{
			using InfoPerChunk = CollisionBoundaryAsBlockInfoPerChunk;

			if (collisionSize.x >= 1.0f || collisionSize.z >= 1.0f)
				return {};

			const Lattice3 worldBlockPositionMin = GetBlockPosition(worldPositionMin);
			const Lattice3 worldBlockPositionMax = GetBlockPosition(worldPositionMin + collisionSize);

			const Lattice2 chunkIndexMin = GetChunkIndex(worldBlockPositionMin);
			const Lattice2 chunkIndexMax = GetChunkIndex(worldBlockPositionMax);

			// 開始地点のチャンクが配列の範囲外
			if (!IsValidChunkIndex(chunkIndexMin, chunkCount))
			{
				return
				{
					InfoPerChunk::CreateUncontained(Lattice2(0, 0)),
					InfoPerChunk::CreateUncontained(Lattice2(1, 0)),
					InfoPerChunk::CreateUncontained(Lattice2(0, 1)),
					InfoPerChunk::CreateUncontained(Lattice2(1, 1)),
				};
			}
			// 以降、開始地点のチャンクには、必ずコリジョンが属する

			// 各チャンクが配列の範囲内か
			// x0z0, x1z0, x0z1, x1z1 の順番
			const bool isValidChunkFlags[4] =
			{
				true,
				MathUtils::IsInRange(chunkIndexMax.x, 0, chunkCount),
				MathUtils::IsInRange(chunkIndexMax.y, 0, chunkCount),
				IsValidChunkIndex(chunkIndexMax, chunkCount)
			};

			// X,Z 方向にチャンクを跨いでいるか
			const bool isCrossingChunkX = chunkIndexMin.x < chunkIndexMax.x;
			const bool isCrossingChunkZ = chunkIndexMin.y < chunkIndexMax.y;

			// そのチャンク内でのローカルブロック座標
			const Lattice3 localBlockPositionMin = GetChunkLocalPosition(worldBlockPositionMin);
			const Lattice3 localBlockPositionMax = GetChunkLocalPosition(worldBlockPositionMax);

			// 2x2 チャンク内でのローカルブロック座標に変換
			// 例えばチャンクが [16, 256, 16] のサイズで、チャンク配列が 2x2 のサイズだったら、
			// 変換前の値の範囲は [0, 0, 0] ~ [15, 255, 15] で、
			// 変換後の値の範囲は [0, 0, 0] ~ [31, 255, 31] である
			// ただし、チャンクがチャンク配列の範囲外である場合、このサイズを超えていることもある
			const Lattice3 localBlockPositionMinIn2x2Chunks = localBlockPositionMin;
			const Lattice3 localBlockPositionMaxIn2x2Chunks = localBlockPositionMax
				+ Lattice3(
					isCrossingChunkX ? chunkSize : 0,
					0,
					isCrossingChunkZ ? chunkSize : 0
				);

			std::array<InfoPerChunk, 4> result = {};
			for (int i = 0; i < 4; ++i)
			{
				// 2x2 のチャンク群内における、このチャンクのローカルインデックスを算出
				const Lattice2 localChunkIndex =
					Lattice2(
						(i & 0b01) ? 1 : 0,
						(i & 0b10) ? 1 : 0
					);

				// そのチャンクが配列の範囲外なので、スキップ
				if (!isValidChunkFlags[i])
				{
					result[i] = InfoPerChunk::CreateUncontained(localChunkIndex);
					continue;
				}

				// コリジョンが属さないチャンクなので、スキップ
				if (
					(i == 1 && !isCrossingChunkX) ||
					(i == 2 && !isCrossingChunkZ) ||
					(i == 3 && !(isCrossingChunkX && isCrossingChunkZ))
					)
				{
					result[i] = InfoPerChunk::CreateUncontained(localChunkIndex);
					continue;
				}

				// 正式にこのチャンクにコリジョンが属するので、値を格納する

				// 2x2 チャンク内でのローカルブロック座標
				const Lattice2 rangeX2x2 = Lattice2(localBlockPositionMinIn2x2Chunks.x, localBlockPositionMaxIn2x2Chunks.x);
				const Lattice2 rangeY2x2 = Lattice2(localBlockPositionMin.y, localBlockPositionMax.y);
				const Lattice2 rangeZ2x2 = Lattice2(localBlockPositionMinIn2x2Chunks.z, localBlockPositionMaxIn2x2Chunks.z);

				// これから、このチャンクにおけるローカルブロック座標に直す
				Lattice2 rangeX;
				const Lattice2 rangeY = rangeY2x2;
				Lattice2 rangeZ;

				// x0,z0
				if (i == 0)
				{
					rangeX = Lattice2(
						rangeX2x2.x,
						std::min(rangeX2x2.y, chunkSize - 1)
					);
					rangeZ = Lattice2(
						rangeZ2x2.x,
						std::min(rangeZ2x2.y, chunkSize - 1)
					);
				}
				// x1,z0
				else if (i == 1)
				{
					rangeX = Lattice2(
						std::max(rangeX2x2.x - chunkSize, 0),
						rangeX2x2.y - chunkSize
					);
					rangeZ = Lattice2(
						rangeZ2x2.x,
						std::min(rangeZ2x2.y, chunkSize - 1)
					);
				}
				// x0,z1
				else if (i == 2)
				{
					rangeX = Lattice2(
						rangeX2x2.x,
						std::min(rangeX2x2.y, chunkSize - 1)
					);
					rangeZ = Lattice2(
						std::max(rangeZ2x2.x - chunkSize, 0),
						rangeZ2x2.y - chunkSize
					);
				}
				// x1,z1
				else // i == 3
				{
					rangeX = Lattice2(
						std::max(rangeX2x2.x - chunkSize, 0),
						rangeX2x2.y - chunkSize
					);
					rangeZ = Lattice2(
						std::max(rangeZ2x2.x - chunkSize, 0),
						rangeZ2x2.y - chunkSize
					);
				}

				result[i] = InfoPerChunk::CreateContained(localChunkIndex, rangeX, rangeY, rangeZ);
			}

			return result;
		}

		/// <summary>
		/// 足元より下である中で、最も高いブロックのY座標を取得する (無いなら チャンクの高さの最小値-1 を返す)
		/// </summary>
		static int FindFloorHeight(
			const Chunk::ChunksArray<Chunk>& chunks, const Vector3& footWorldPosition, const Vector3& collisionSize,
			int chunkSize = Chunk::Size, int chunkCount = Chunk::Count)
		{
			const auto& [info, infoX, infoZ, infoXZ] =
				CalculateCollisionBoundaryAsBlock(GetCollisionMinPosition(footWorldPosition, collisionSize), collisionSize, chunkSize, chunkCount);

			const std::function<int(const CollisionBoundaryAsBlockInfoPerChunk&)> FindFloorHeightForThisChunk =
				[&chunks, &footWorldPosition, &collisionSize, chunkCount](const auto& info)
				{
					// 情報をアンパック
					const auto& [isInsideChunk, localChunkIndex, rangeX, rangeY, rangeZ] = info;

					// このチャンクにコリジョンが属していないので、最小値を返す
					if (!isInsideChunk)
						return -1;

					// チャンクを取得 (配列の範囲外なら、最小値を返す)
					const Lattice2 chunkIndex = GetChunkIndex(GetBlockPosition(GetCollisionMinPosition(footWorldPosition, collisionSize))) + localChunkIndex;
					if (!IsValidChunkIndex(chunkIndex, chunkCount))
						return -1;
					const Chunk& chunk = chunks[chunkIndex.x][chunkIndex.y];

					int y = -1;
					for (int x = rangeX.x; x <= rangeX.y; ++x)
						for (int z = rangeZ.x; z <= rangeZ.y; ++z)
						{
							const int height = chunk.GetFloorHeight({ x, z }, rangeY.x - 1);
							y = std::max(y, height);
						}

					return y;
				};

			int y = -1;
			y = std::max(y, FindFloorHeightForThisChunk(info));
			y = std::max(y, FindFloorHeightForThisChunk(infoX));
			y = std::max(y, FindFloorHeightForThisChunk(infoZ));
			y = std::max(y, FindFloorHeightForThisChunk(infoXZ));
			return y;
		}

		/// <summary>
		/// 頭上より上である中で、最も低いブロックのY座標を取得する (無いなら チャンクの高さの最大値+1 を返す)
		/// </summary>
		static int FindCeilHeight(
			const Chunk::ChunksArray<Chunk>& chunks, const Vector3& footWorldPosition, const Vector3& collisionSize,
			int chunkSize = Chunk::Size, int chunkHeight = Chunk::Height, int chunkCount = Chunk::Count)
		{
			const auto& [info, infoX, infoZ, infoXZ] =
				CalculateCollisionBoundaryAsBlock(GetCollisionMinPosition(footWorldPosition, collisionSize), collisionSize, chunkSize, chunkCount);

			const std::function<int(const CollisionBoundaryAsBlockInfoPerChunk&)> FindCeilHeightForThisChunk =
				[&chunks, &footWorldPosition, &collisionSize, chunkHeight, chunkCount](const auto& info)
				{
					// 情報をアンパック
					const auto& [isInsideChunk, localChunkIndex, rangeX, rangeY, rangeZ] = info;

					// このチャンクにコリジョンが属していないので、最大値を返す
					if (!isInsideChunk)
						return chunkHeight;

					// チャンクを取得 (配列の範囲外なら、最大値を返す)
					const Lattice2 chunkIndex = GetChunkIndex(GetBlockPosition(GetCollisionMinPosition(footWorldPosition, collisionSize))) + localChunkIndex;
					if (!IsValidChunkIndex(chunkIndex, chunkCount))
						return chunkHeight;
					const Chunk& chunk = chunks[chunkIndex.x][chunkIndex.y];

					int y = chunkHeight;
					for (int x = rangeX.x; x <= rangeX.y; ++x)
						for (int z = rangeZ.x; z <= rangeZ.y; ++z)
						{
							const int height = chunk.GetCeilHeight({ x, z }, rangeY.y + 1);
							y = std::min(y, height);
						}

					return y;
				};

			int y = chunkHeight;
			y = std::min(y, FindCeilHeightForThisChunk(info));
			y = std::min(y, FindCeilHeightForThisChunk(infoX));
			y = std::min(y, FindCeilHeightForThisChunk(infoZ));
			y = std::min(y, FindCeilHeightForThisChunk(infoXZ));
			return y;
		}

		/// <summary>
		/// 地形との当たり判定
		/// </summary>
		static bool IsOverlappingWithTerrain(
			const Chunk::ChunksArray<Chunk>& chunks, const Vector3& footWorldPosition, const Vector3& collisionSize,
			int chunkSize = Chunk::Size, int chunkCount = Chunk::Count)
		{
			const auto& [info, infoX, infoZ, infoXZ] =
				CalculateCollisionBoundaryAsBlock(GetCollisionMinPosition(footWorldPosition, collisionSize), collisionSize, chunkSize, chunkCount);

			const std::function<bool(const CollisionBoundaryAsBlockInfoPerChunk&)> IsOverlappingForThisChunk =
				[&chunks, &footWorldPosition, &collisionSize, chunkCount](const auto& info)
				{
					// 情報をアンパック
					const auto& [isInsideChunk, localChunkIndex, rangeX, rangeY, rangeZ] = info;

					// このチャンクにコリジョンが属していないので、重なっていない
					if (!isInsideChunk)
						return false;

					// チャンクを取得 (配列の範囲外なら、重なっていないとみなす)
					const Lattice2 chunkIndex = GetChunkIndex(GetBlockPosition(GetCollisionMinPosition(footWorldPosition, collisionSize))) + localChunkIndex;
					if (!IsValidChunkIndex(chunkIndex, chunkCount))
						return false;
					const Chunk& chunk = chunks[chunkIndex.x][chunkIndex.y];

					for (int x = rangeX.x; x <= rangeX.y; ++x)
						for (int y = rangeY.x; y <= rangeY.y; ++y)
							for (int z = rangeZ.x; z <= rangeZ.y; ++z)
							{
								if (chunk.GetBlock({ x, y, z }) != Block::Air)
									return true;
							}

					return false;
				};

			if (IsOverlappingForThisChunk(info)) return true;
			if (IsOverlappingForThisChunk(infoX)) return true;
			if (IsOverlappingForThisChunk(infoZ)) return true;
			if (IsOverlappingForThisChunk(infoXZ)) return true;
			return false;
		}
	};
}
