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
		static Lattice2 GetChunkIndex(const Lattice3& worldBlockPosition)
		{
			return Lattice2(
				worldBlockPosition.x / Terrain::ChunkSize,
				worldBlockPosition.z / Terrain::ChunkSize
			);
		}

		// チャンク内のローカルブロック座標に変換
		static Lattice3 GetChunkLocalPosition(const Lattice3& worldBlockPosition)
		{
			return Lattice3(
				worldBlockPosition.x % Terrain::ChunkSize,
				worldBlockPosition.y,
				worldBlockPosition.z % Terrain::ChunkSize
			);
		}

		// チャンクインデックスが有効であるか
		// 即ち、上下限を超えた値でないか
		static bool IsValidChunkIndex(const Lattice2& chunkIndex, int chunkCount)
		{
			return 0 <= chunkIndex.x && chunkIndex.x < chunkCount
				&& 0 <= chunkIndex.y && chunkIndex.y < chunkCount;
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

			if (std::abs((newRotation * Vector3::Forward()).y) < 0.99f) // 上下回転を制限 (前方向ベクトルのy成分で判定)
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

		/// <summary>
		/// <para>コリジョン立方体の範囲が、どのブロック座標に属するかを計算する</para>
		/// <para>複数チャンクに跨っている場合も考慮する</para>
		/// <para>コリジョンのxz方向のサイズは、1より小さい想定!</para>
		/// <para>[戻り値の形式]</para>
		/// <para>最小インデックスのチャンク, X方向に1つ進んだチャンク, Z方向に1つ進んだチャンク, XZ両方向に1つ進んだチャンク の順に情報を返す</para>
		/// <para>各チャンクについて、(コリジョン立方体が属するか, チャンクインデックス, X方向の範囲, Y方向の範囲, Z方向の範囲) のタプルを返す</para>
		/// <para>範囲は、そのチャンクにおけるローカルブロック座標</para>
		/// <para>そのチャンクに属さない場合、範囲はデフォルト値</para>
		/// </summary>
		template <int ChunkCount>
		static std::array<std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>, 4> CalculateCollisionBoundaryAsBlock(
			const std::array<std::array<Terrain, ChunkCount>, ChunkCount>& terrainChunks,
			const Vector3& worldPositionMin, const Vector3& collisionSize)
		{
			if (collisionSize.x >= 1.0f || collisionSize.z >= 1.0f)
				return {};

			const Lattice3 worldBlockPositionMin = GetBlockPosition(worldPositionMin);
			const Lattice3 worldBlockPositionMax = GetBlockPosition(worldPositionMin + collisionSize);

			const Lattice2 chunkIndexMin = GetChunkIndex(worldBlockPositionMin);
			const Lattice2 chunkIndexMax = GetChunkIndex(worldBlockPositionMax);

			const Lattice3 localBlockPositionMin = GetChunkLocalPosition(worldBlockPositionMin); // 最小座標の、そのチャンク内でのローカル座標
			const Lattice3 localBlockPositionMax = GetChunkLocalPosition(worldBlockPositionMax); // 最大座標の、そのチャンク内でのローカル座標

			// 各チャンクに存在するか
			const bool isInsideChunkFlags[4] =
			{
				IsValidChunkIndex(chunkIndexMin, ChunkCount),
				(chunkIndexMin.x < chunkIndexMax.x),
				(chunkIndexMin.y < chunkIndexMax.y),
				(chunkIndexMin.x < chunkIndexMax.x) && (chunkIndexMin.y < chunkIndexMax.y)
			};

			std::array<std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>, 4> result = {};
			for (int i = 0; i < 4; ++i)
			{
				// 最初に取得した 各 min,max の値を "このチャンクにおける" 数値に変換
				const Lattice2 chunkIndex =
					Lattice2(
						chunkIndexMin.x + ((i & 0b01) ? 1 : 0),
						chunkIndexMin.y + ((i & 0b10) ? 1 : 0)
					);
				const Lattice3 localBlockPositionStart =
					Lattice3(
						(i & 0b01) ? 0 : localBlockPositionMin.x,
						localBlockPositionMin.y,
						(i & 0b10) ? 0 : localBlockPositionMin.z
					);
				const Lattice3 localBlockPositionLast =
					Lattice3(
						(i & 0b01) ? localBlockPositionMax.x : (Terrain::ChunkSize - 1),
						localBlockPositionMax.y,
						(i & 0b10) ? localBlockPositionMax.z : (Terrain::ChunkSize - 1)
					);

				// そのチャンクにコリジョンが属さないので、スキップ
				if (!isInsideChunkFlags[i])
				{
					result[i] = std::make_tuple(false, chunkIndex, Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero());
					continue;
				}

				// チャンクインデックスが配列のサイズを超えてしまったので、スキップ
				if (!IsValidChunkIndex(chunkIndex, ChunkCount))
				{
					result[i] = std::make_tuple(false, chunkIndex, Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero());
					continue;
				}

				// 正式にこのチャンクにコリジョンが属するので、値を格納する
				result[i] = std::make_tuple(
					true,
					chunkIndex,
					Lattice2(localBlockPositionStart.x, localBlockPositionLast.x),
					Lattice2(localBlockPositionStart.y, localBlockPositionLast.y),
					Lattice2(localBlockPositionStart.z, localBlockPositionLast.z)
				);
			}

			return result;
		}

		// 足元より下である中で、最も高いブロックのY座標を取得する (無いなら チャンクの高さの最小値-1 を返す)
		template<int ChunkCount>
		static int FindFloorHeight(
			const std::array<std::array<Terrain, ChunkCount>, ChunkCount>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const auto& [info, infoX, infoZ, infoXZ] =
				CalculateCollisionBoundaryAsBlock(terrainChunks, GetCollisionMinPosition(position, size), size);

			const std::function<int(std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>)> FindFloorHeightForThisChunk =
				[&terrainChunks, &position](std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2> boundaryInfo) -> int
				{
					// 情報をアンパック
					const auto& [isInsideChunk, chunkIndex, rangeX, rangeY, rangeZ] = boundaryInfo;

					// このチャンクにコリジョンが属していないので、最小値を返す
					if (!isInsideChunk)
						return -1;

					// 配列のサイズ外チェックは事前に済んでいるはず
					const Terrain& chunk = terrainChunks[chunkIndex.x][chunkIndex.y];

					int y = -1;
					for (int z = rangeZ.x; z <= rangeZ.y; ++z)
						for (int x = rangeX.x; x <= rangeX.y; ++x)
						{
							const int height = chunk.GetFloorHeight(x, z, rangeY.x - 1);
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

		// 頭上より上である中で、最も低いブロックのY座標を取得する (無いなら チャンクの高さの最大値+1 を返す)
		template<int ChunkCount>
		static int FindCeilHeight(
			const std::array<std::array<Terrain, ChunkCount>, ChunkCount>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const auto& [info, infoX, infoZ, infoXZ] =
				CalculateCollisionBoundaryAsBlock(terrainChunks, GetCollisionMinPosition(position, size), size);

			const std::function<int(std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>)> FindCeilHeightForThisChunk =
				[&terrainChunks, &position](std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2> boundaryInfo) -> int
				{
					// 情報をアンパック
					const auto& [isInsideChunk, chunkIndex, rangeX, rangeY, rangeZ] = boundaryInfo;

					// このチャンクにコリジョンが属していないので、最大値を返す
					if (!isInsideChunk)
						return Terrain::ChunkHeight;

					// 配列のサイズ外チェックは事前に済んでいるはず
					const Terrain& chunk = terrainChunks[chunkIndex.x][chunkIndex.y];

					int y = Terrain::ChunkHeight;
					for (int z = rangeZ.x; z <= rangeZ.y; ++z)
						for (int x = rangeX.x; x <= rangeX.y; ++x)
						{
							const int height = chunk.GetCeilHeight(x, z, rangeY.y + 1);
							y = std::min(y, height);
						}

					return y;
				};

			int y = Terrain::ChunkHeight;
			y = std::min(y, FindCeilHeightForThisChunk(info));
			y = std::min(y, FindCeilHeightForThisChunk(infoX));
			y = std::min(y, FindCeilHeightForThisChunk(infoZ));
			y = std::min(y, FindCeilHeightForThisChunk(infoXZ));
			return y;
		}

		template<int ChunkCount>
		static bool IsOverlappingWithTerrain(
			const std::array<std::array<Terrain, ChunkCount>, ChunkCount>& terrainChunks,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const auto& [info, infoX, infoZ, infoXZ] =
				CalculateCollisionBoundaryAsBlock(terrainChunks, GetCollisionMinPosition(position, size), size);

			const std::function<bool(std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>)> IsOverlappingForThisChunk =
				[&terrainChunks](std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2> boundaryInfo) -> bool
				{
					// 情報をアンパック
					const auto& [isInsideChunk, chunkIndex, rangeX, rangeY, rangeZ] = boundaryInfo;

					// このチャンクにコリジョンが属していないので、重なっていない
					if (!isInsideChunk)
						return false;

					// 配列のサイズ外チェックは事前に済んでいるはず
					const Terrain& chunk = terrainChunks[chunkIndex.x][chunkIndex.y];

					for (int y = rangeY.x; y <= rangeY.y; ++y)
						for (int z = rangeZ.x; z <= rangeZ.y; ++z)
							for (int x = rangeX.x; x <= rangeX.y; ++x)
							{
								if (chunk.GetBlock(x, y, z) != Block::Air)
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

// テスト
#if _DEBUG

#include <cassert>

namespace ForiverEngine
{
	struct Test_PlayerControl
	{
		static void RunAll()
		{
			GetBlockPosition::RunAll();
		}

		struct GetBlockPosition
		{
			static void RunAll()
			{
				Run_GetBlockPosition_1();
				Run_GetBlockPosition_3();

				Run_GetChunkIndex();
				Run_GetChunkLocalPosition();
				Run_IsValidChunkIndex();

				Run_GetFootPosition();
				Run_GetCollisionMinPosition();

				Run_Rotate();
				Run_MoveH();
			}

			static void Run_GetBlockPosition_1()
			{
				assert(PlayerControl::GetBlockPosition(0.0f) == 0);
				assert(PlayerControl::GetBlockPosition(0.4f) == 0);
				assert(PlayerControl::GetBlockPosition(0.5f) == 1);
				assert(PlayerControl::GetBlockPosition(0.6f) == 1);
				assert(PlayerControl::GetBlockPosition(1.0f) == 1);
				assert(PlayerControl::GetBlockPosition(-0.4f) == 0);
				assert(PlayerControl::GetBlockPosition(-0.5f) == -1);
			}

			static void Run_GetBlockPosition_3()
			{
				assert(PlayerControl::GetBlockPosition(Vector3(0.0f, 1.0f, 2.0f)) == Lattice3(0, 1, 2));
			}

			static void Run_GetChunkIndex()
			{
				assert(PlayerControl::GetChunkIndex(Lattice3(0, 0, 0)) == Lattice2(0, 0));
				assert(PlayerControl::GetChunkIndex(Lattice3(15, 0, 15)) == Lattice2(0, 0));
				assert(PlayerControl::GetChunkIndex(Lattice3(16, 0, 0)) == Lattice2(1, 0));
				assert(PlayerControl::GetChunkIndex(Lattice3(0, 0, 16)) == Lattice2(0, 1));
				assert(PlayerControl::GetChunkIndex(Lattice3(16, 0, 16)) == Lattice2(1, 1));
			}

			static void Run_GetChunkLocalPosition()
			{
				assert(PlayerControl::GetChunkLocalPosition(Lattice3(0, 0, 0)) == Lattice3(0, 0, 0));
				assert(PlayerControl::GetChunkLocalPosition(Lattice3(15, 0, 15)) == Lattice3(15, 0, 15));
				assert(PlayerControl::GetChunkLocalPosition(Lattice3(16, 0, 0)) == Lattice3(0, 0, 0));
				assert(PlayerControl::GetChunkLocalPosition(Lattice3(17, 0, 0)) == Lattice3(1, 0, 0));
				assert(PlayerControl::GetChunkLocalPosition(Lattice3(0, 0, 16)) == Lattice3(0, 0, 0));
				assert(PlayerControl::GetChunkLocalPosition(Lattice3(0, 0, 17)) == Lattice3(0, 0, 1));
				assert(PlayerControl::GetChunkLocalPosition(Lattice3(16, 0, 16)) == Lattice3(0, 0, 0));
			}

			static void Run_IsValidChunkIndex()
			{
				assert(PlayerControl::IsValidChunkIndex(Lattice2(0, 0), 4) == true);
				assert(PlayerControl::IsValidChunkIndex(Lattice2(3, 3), 4) == true);
				assert(PlayerControl::IsValidChunkIndex(Lattice2(-1, 0), 4) == false);
				assert(PlayerControl::IsValidChunkIndex(Lattice2(0, -1), 4) == false);
				assert(PlayerControl::IsValidChunkIndex(Lattice2(4, 0), 4) == false);
				assert(PlayerControl::IsValidChunkIndex(Lattice2(0, 4), 4) == false);
			}

			static void Run_GetFootPosition()
			{
				assert(PlayerControl::GetFootPosition(Vector3(0.0f, 1.8f, 0.0f), 1.8f) == Vector3(0.0f, 0.0f, 0.0f));
				assert(PlayerControl::GetFootPosition(Vector3(1.0f, 2.5f, -3.0f), 1.5f) == Vector3(1.0f, 1.0f, -3.0f));
			}

			static void Run_GetCollisionMinPosition()
			{
				assert(PlayerControl::GetCollisionMinPosition(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.6f, 1.8f, 0.6f)) == Vector3(-0.3f, 0.0f, -0.3f));
				assert(PlayerControl::GetCollisionMinPosition(Vector3(1.0f, 2.0f, 3.0f), Vector3(1.0f, 2.0f, 1.0f)) == Vector3(0.5f, 2.0f, 2.5f));
			}

			static void Run_Rotate()
			{
				struct _
				{
					static Quaternion Calc(const Vector3& position, const Quaternion& rotation, const Vector2& input, const Vector2& speed, float dt)
					{
						const Transform transform = { .position = position, .rotation = rotation };
						return PlayerControl::Rotate(transform, input, speed, dt);
					}
				};

				// 入力無し
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2::Zero(),
					Vector2(90.0f, 90.0f) * DegToRad,
					1.0f
				) == Quaternion::Identity());

				// Yaw 90度回転
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(1.0f, 0.0f),
					Vector2(90.0f, 90.0f) * DegToRad,
					1.0f
				) == Quaternion::FromAxisAngle(Vector3::Up(), 90.0f * DegToRad));

				// Pitch 45度回転
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.0f, 1.0f),
					Vector2(90.0f, 90.0f) * DegToRad,
					0.5f
				) == Quaternion::FromAxisAngle(Vector3::Right(), -45.0f * DegToRad));

				// Pitch 上限を超える回転
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::FromAxisAngle(Vector3::Right(), -85.0f * DegToRad),
					Vector2(0.0f, 1.0f),
					Vector2(4.0f, 4.0f) * DegToRad,
					1.0f
				) == Quaternion::FromAxisAngle(Vector3::Right(), -85.0f * DegToRad));

				// Pitch 下限を超える回転 
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::FromAxisAngle(Vector3::Right(), 85.0f * DegToRad),
					Vector2(0.0f, -1.0f),
					Vector2(4.0f, 4.0f) * DegToRad,
					1.0f
				) == Quaternion::FromAxisAngle(Vector3::Right(), 85.0f * DegToRad));

				// Pitch 上限を超える回転 (回転後が上下90度を超える)
				// これは失敗するはず
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.0f, 1.0f),
					Vector2(90.0f, 90.0f) * DegToRad,
					2.0f
				) != Quaternion::Identity());

				// Yaw と Pitch の同時回転
				// Yaw 45度, Pitch -45度
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.5f, -0.5f),
					Vector2(90.0f, 90.0f) * DegToRad,
					1.0f
				) == Quaternion::FromAxisAngle(Vector3::Up(), 45.0f * DegToRad) *
					Quaternion::FromAxisAngle(Vector3::Right(), 45.0f * DegToRad));

#undef ASSERT
			}

			static void Run_MoveH()
			{
				struct _
				{
					static Vector3 Calc(const Vector3& position, const Quaternion& rotation, const Vector2& input, float speed, float dt)
					{
						const Transform transform = { .position = position, .rotation = rotation };
						return PlayerControl::MoveH(transform, input, speed, dt);
					}
				};

				// 入力無し
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2::Zero(),
					1.0f,
					1.0f
				) == Vector3::Zero());

				// 前進
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.0f, 1.0f),
					2.0f,
					1.0f
				) == Vector3(0.0f, 0.0f, 2.0f));

				// 右移動
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(1.0f, 0.0f),
					3.0f,
					1.0f
				) == Vector3(3.0f, 0.0f, 0.0f));

				// 斜め移動
				assert(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(1.0f, 1.0f),
					std::sqrt(2.0f),
					1.0f
				) == Vector3(1.0f, 0.0f, 1.0f));

#undef ASSERT
			}
		};
	};
}

#endif
