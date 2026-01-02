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

			// そのチャンク内でのローカルブロック座標
			const Lattice3 localBlockPositionMin = GetChunkLocalPosition(worldBlockPositionMin);
			const Lattice3 localBlockPositionMax = GetChunkLocalPosition(worldBlockPositionMax);

			// 各チャンクに存在するか
			const bool isInsideChunkFlags[4] =
			{
				IsValidChunkIndex(chunkIndexMin, ChunkCount),
				(chunkIndexMin.x < chunkIndexMax.x),
				(chunkIndexMin.y < chunkIndexMax.y),
				(chunkIndexMin.x < chunkIndexMax.x) && (chunkIndexMin.y < chunkIndexMax.y)
			};

			// 2x2 チャンク内でのローカルブロック座標に変換
			// 例えばチャンクが [16, 256, 16] のサイズだったら、
			// 変換前の値の範囲は [0, 0, 0] ~ [15, 255, 15] で、
			// 変換後の値の範囲は [0, 0, 0] ~ [31, 255, 31] である
			const Lattice3 localBlockPositionMinIn2x2Chunks = localBlockPositionMin;
			const Lattice3 localBlockPositionMaxIn2x2Chunks = localBlockPositionMax
				+ Lattice3(
					isInsideChunkFlags[1] ? Terrain::ChunkSize : 0,
					0,
					isInsideChunkFlags[2] ? Terrain::ChunkSize : 0
				);

			std::array<std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>, 4> result = {};
			for (int i = 0; i < 4; ++i)
			{
				// 最初に取得した 各 min,max の値を "このチャンクにおける" 数値に変換
				const Lattice2 chunkIndex =
					Lattice2(
						chunkIndexMin.x + ((i & 0b01) ? 1 : 0),
						chunkIndexMin.y + ((i & 0b10) ? 1 : 0)
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
						std::min(rangeX2x2.y, Terrain::ChunkSize - 1)
					);
					rangeZ = Lattice2(
						rangeZ2x2.x,
						std::min(rangeZ2x2.y, Terrain::ChunkSize - 1)
					);
				}
				// x1,z0
				else if (i == 1)
				{
					rangeX = Lattice2(
						std::max(rangeX2x2.x - Terrain::ChunkSize, 0),
						rangeX2x2.y - Terrain::ChunkSize
					);
					rangeZ = Lattice2(
						rangeZ2x2.x,
						std::min(rangeZ2x2.y, Terrain::ChunkSize - 1)
					);
				}
				// x0,z1
				else if (i == 2)
				{
					rangeX = Lattice2(
						rangeX2x2.x,
						std::min(rangeX2x2.y, Terrain::ChunkSize - 1)
					);
					rangeZ = Lattice2(
						std::max(rangeZ2x2.x - Terrain::ChunkSize, 0),
						rangeZ2x2.y - Terrain::ChunkSize
					);
				}
				// x1,z1
				else // i == 3
				{
					rangeX = Lattice2(
						std::max(rangeX2x2.x - Terrain::ChunkSize, 0),
						rangeX2x2.y - Terrain::ChunkSize
					);
					rangeZ = Lattice2(
						std::max(rangeZ2x2.x - Terrain::ChunkSize, 0),
						rangeZ2x2.y - Terrain::ChunkSize
					);
				}

				result[i] = std::make_tuple(true, chunkIndex, rangeX, rangeY, rangeZ);
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
	private:

#pragma region Chunk Helpers

		struct CollisionBoundaryInfoWrapper
		{
			std::array<std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>, 4> value{};

			bool operator==(const CollisionBoundaryInfoWrapper& other) const noexcept
			{
				return value == other.value;
			}
			bool operator!=(const CollisionBoundaryInfoWrapper& other) const noexcept
			{
				return !(*this == other);
			}

			std::string ToString() const noexcept
			{
				std::string text = "\n{";
				for (const auto& info : value)
				{
					const auto& [isExistInChunk, chunkIndex__, rangeX, rangeY, rangeZ] = info;
					text += std::format(
						"[{},{},{},{}],",
						isExistInChunk ? "o" : "x",
						(isExistInChunk || rangeX != Lattice2::Zero()) ? ForiverEngine::ToString(rangeX) : "_",
						(isExistInChunk || rangeY != Lattice2::Zero()) ? ForiverEngine::ToString(rangeY) : "_",
						(isExistInChunk || rangeZ != Lattice2::Zero()) ? ForiverEngine::ToString(rangeZ) : "_"
					);
				}
				text += "}";

				return text;
			}
		};

		// チャンクを手動作成
		// y[0, Terrain::ChunkHeight-1] の各層が存在するかどうかを layers で指定
		inline static Terrain CreateChunk(const std::array<bool, Terrain::ChunkHeight>& layers)
		{
			Terrain chunk = Terrain::CreateVoid(
				Terrain::ChunkSize, Terrain::ChunkHeight, Terrain::ChunkSize);
			for (int y = 0; y < Terrain::ChunkHeight; ++y)
			{
				if (layers[y])
				{
					for (int z = 0; z < Terrain::ChunkSize; ++z)
						for (int x = 0; x < Terrain::ChunkSize; ++x)
						{
							chunk.SetBlock(x, y, z, Block::Stone);
						}
				}
			}

			return chunk;
		}

		// ブロック、空気、ブロックの 3層構造のチャンクを作成
		inline static Terrain CreateChunk_Block_Air_Block(const Lattice2& airYRange)
		{
			std::array<bool, Terrain::ChunkHeight> layers = {};
			for (int y = 0; y < Terrain::ChunkHeight; ++y)
			{
				if (y < airYRange.x || airYRange.y < y)
					layers[y] = true;
			}

			return CreateChunk(layers);
		}

		// 2x2のチャンク群を作成
		// ブロック、空気、ブロックの 3層構造
		// min, x隣, z隣, xz隣 の順に airYRanges を指定
		inline static std::array<std::array<Terrain, 2>, 2> CreateChunks(const std::array<Lattice2, 4 >& airYRanges)
		{
			std::array<std::array<Terrain, 2>, 2>chunks = {};

			chunks[0][0] = CreateChunk_Block_Air_Block(airYRanges[0]);
			chunks[1][0] = CreateChunk_Block_Air_Block(airYRanges[1]);
			chunks[0][1] = CreateChunk_Block_Air_Block(airYRanges[2]);
			chunks[1][1] = CreateChunk_Block_Air_Block(airYRanges[3]);

			return chunks;
		}

#pragma endregion

#pragma region Define Custom Assert Methods

		template<typename T>
		static void Assert(const T& value, const T& expected, const std::string& fileName, int lineNumber)
		{
			const std::string errorMessage = std::format("Test failed\nValue: {}\nExpected: {}", value, expected);
			_wassert(
				StringUtils::UTF8ToUTF16(errorMessage).c_str(),
				StringUtils::UTF8ToUTF16(fileName).c_str(),
				static_cast<unsigned>(lineNumber)
			);
		}

		// 基本
#define eq(value, expected) { if ((value) != (expected)) ForiverEngine::Test_PlayerControl::Assert((value), (expected), __FILE__, __LINE__); }
#define neq(value, expected) { if ((value) == (expected)) ForiverEngine::Test_PlayerControl::Assert((value), (expected), __FILE__, __LINE__); }

		// 線形代数 (Linear Algebra)
#define eqla(value, expected) { if ((value) != (expected)) ForiverEngine::Test_PlayerControl::Assert((ToString(value)), (ToString(expected)), __FILE__, __LINE__); }
#define neqla(value, expected) { if ((value) == (expected)) ForiverEngine::Test_PlayerControl::Assert((ToString(value)), (ToString(expected)), __FILE__, __LINE__); }

		// 自作オブジェクト (ToString メソッドを持つ)
#define eqobj(value, expected) { if ((value) != (expected)) ForiverEngine::Test_PlayerControl::Assert((value).ToString(), (expected).ToString(), __FILE__, __LINE__); }
#define neqobj(value, expected) { if ((value) == (expected)) ForiverEngine::Test_PlayerControl::Assert((value).ToString(), (expected).ToString(), __FILE__, __LINE__); }

#pragma endregion

	public:
		static void RunAll()
		{
			GetBlockPosition::RunAll();
		}
	private:

		struct GetBlockPosition
		{
		public:
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

				Run_CalculateCollisionBoundaryAsBlock();
			}
		private:

#pragma region Primitive Calculations

			static void Run_GetBlockPosition_1()
			{
				eq(PlayerControl::GetBlockPosition(0.0f), 0);
				eq(PlayerControl::GetBlockPosition(0.4f), 0);
				eq(PlayerControl::GetBlockPosition(0.5f), 1);
				eq(PlayerControl::GetBlockPosition(0.6f), 1);
				eq(PlayerControl::GetBlockPosition(1.0f), 1);
				eq(PlayerControl::GetBlockPosition(-0.4f), 0);
				eq(PlayerControl::GetBlockPosition(-0.5f), -1);
			}

			static void Run_GetBlockPosition_3()
			{
				eqla(PlayerControl::GetBlockPosition(Vector3(0.0f, 1.0f, 2.0f)), Lattice3(0, 1, 2));
			}

			static void Run_GetChunkIndex()
			{
				eqla(PlayerControl::GetChunkIndex(Lattice3(0, 0, 0)), Lattice2(0, 0));
				eqla(PlayerControl::GetChunkIndex(Lattice3(15, 0, 15)), Lattice2(0, 0));
				eqla(PlayerControl::GetChunkIndex(Lattice3(16, 0, 0)), Lattice2(1, 0));
				eqla(PlayerControl::GetChunkIndex(Lattice3(0, 0, 16)), Lattice2(0, 1));
				eqla(PlayerControl::GetChunkIndex(Lattice3(16, 0, 16)), Lattice2(1, 1));
			}

			static void Run_GetChunkLocalPosition()
			{
				eqla(PlayerControl::GetChunkLocalPosition(Lattice3(0, 0, 0)), Lattice3(0, 0, 0));
				eqla(PlayerControl::GetChunkLocalPosition(Lattice3(15, 0, 15)), Lattice3(15, 0, 15));
				eqla(PlayerControl::GetChunkLocalPosition(Lattice3(16, 0, 0)), Lattice3(0, 0, 0));
				eqla(PlayerControl::GetChunkLocalPosition(Lattice3(17, 0, 0)), Lattice3(1, 0, 0));
				eqla(PlayerControl::GetChunkLocalPosition(Lattice3(0, 0, 16)), Lattice3(0, 0, 0));
				eqla(PlayerControl::GetChunkLocalPosition(Lattice3(0, 0, 17)), Lattice3(0, 0, 1));
				eqla(PlayerControl::GetChunkLocalPosition(Lattice3(16, 0, 16)), Lattice3(0, 0, 0));
			}

			static void Run_IsValidChunkIndex()
			{
				eq(PlayerControl::IsValidChunkIndex(Lattice2(0, 0), 4), true);
				eq(PlayerControl::IsValidChunkIndex(Lattice2(3, 3), 4), true);
				eq(PlayerControl::IsValidChunkIndex(Lattice2(-1, 0), 4), false);
				eq(PlayerControl::IsValidChunkIndex(Lattice2(0, -1), 4), false);
				eq(PlayerControl::IsValidChunkIndex(Lattice2(4, 0), 4), false);
				eq(PlayerControl::IsValidChunkIndex(Lattice2(0, 4), 4), false);
			}

			static void Run_GetFootPosition()
			{
				eqla(PlayerControl::GetFootPosition(Vector3(0.0f, 1.8f, 0.0f), 1.8f), Vector3(0.0f, 0.0f, 0.0f));
				eqla(PlayerControl::GetFootPosition(Vector3(1.0f, 2.5f, -3.0f), 1.5f), Vector3(1.0f, 1.0f, -3.0f));
			}

			static void Run_GetCollisionMinPosition()
			{
				eqla(PlayerControl::GetCollisionMinPosition(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.6f, 1.8f, 0.6f)), Vector3(-0.3f, 0.0f, -0.3f));
				eqla(PlayerControl::GetCollisionMinPosition(Vector3(1.0f, 2.0f, 3.0f), Vector3(1.0f, 2.0f, 1.0f)), Vector3(0.5f, 2.0f, 2.5f));
			}

#pragma endregion

#pragma region Player Movements

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
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2::Zero(),
					Vector2(90.0f, 90.0f) * DegToRad,
					1.0f
				), Quaternion::Identity());

				// Yaw 90度回転
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(1.0f, 0.0f),
					Vector2(90.0f, 90.0f) * DegToRad,
					1.0f
				), Quaternion::FromAxisAngle(Vector3::Up(), 90.0f * DegToRad));

				// Pitch 45度回転
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.0f, 1.0f),
					Vector2(90.0f, 90.0f) * DegToRad,
					0.5f
				), Quaternion::FromAxisAngle(Vector3::Right(), -45.0f * DegToRad));

				// Pitch 上限を超える回転
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::FromAxisAngle(Vector3::Right(), -85.0f * DegToRad),
					Vector2(0.0f, 1.0f),
					Vector2(4.0f, 4.0f) * DegToRad,
					1.0f
				), Quaternion::FromAxisAngle(Vector3::Right(), -85.0f * DegToRad));

				// Pitch 下限を超える回転 
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::FromAxisAngle(Vector3::Right(), 85.0f * DegToRad),
					Vector2(0.0f, -1.0f),
					Vector2(4.0f, 4.0f) * DegToRad,
					1.0f
				), Quaternion::FromAxisAngle(Vector3::Right(), 85.0f * DegToRad));

				// Pitch 上限を超える回転 (回転後が上下90度を超える)
				// これは失敗するはず
				neqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.0f, 1.0f),
					Vector2(90.0f, 90.0f) * DegToRad,
					2.0f
				), Quaternion::Identity());

				// Yaw と Pitch の同時回転
				// Yaw 45度, Pitch -45度
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.5f, -0.5f),
					Vector2(90.0f, 90.0f) * DegToRad,
					1.0f
				), Quaternion::FromAxisAngle(Vector3::Up(), 45.0f * DegToRad) *
					Quaternion::FromAxisAngle(Vector3::Right(), 45.0f * DegToRad));
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
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2::Zero(),
					1.0f,
					1.0f
				), Vector3::Zero());

				// 前進
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(0.0f, 1.0f),
					2.0f,
					1.0f
				), Vector3(0.0f, 0.0f, 2.0f));

				// 右移動
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(1.0f, 0.0f),
					3.0f,
					1.0f
				), Vector3(3.0f, 0.0f, 0.0f));

				// 斜め移動
				eqla(_::Calc(
					Vector3::Zero(),
					Quaternion::Identity(),
					Vector2(1.0f, 1.0f),
					std::sqrt(2.0f),
					1.0f
				), Vector3(1.0f, 0.0f, 1.0f));
			}

#pragma endregion

			static void Run_CalculateCollisionBoundaryAsBlock()
			{
#define test( \
	worldPositionMin, \
	b00, x00, y00, z00, \
	b10, x10, y10, z10, \
	b01, x01, y01, z01, \
	b11, x11, y11, z11 \
)  \
{ \
	eqobj(CollisionBoundaryInfoWrapper(PlayerControl::CalculateCollisionBoundaryAsBlock( \
		chunks, \
		Vector3##worldPositionMin, \
		CollisionSize \
	)), CollisionBoundaryInfoWrapper(std::array<std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>, 4> \
	{ \
		std::make_tuple(##b00, Lattice2(0, 0), Lattice2##x00, Lattice2##y00, Lattice2##z00), \
		std::make_tuple(##b10, Lattice2(1, 0), Lattice2##x10, Lattice2##y10, Lattice2##z10), \
		std::make_tuple(##b01, Lattice2(0, 1), Lattice2##x01, Lattice2##y01, Lattice2##z01), \
		std::make_tuple(##b11, Lattice2(1, 1), Lattice2##x11, Lattice2##y11, Lattice2##z11), \
	})); \
} \

				constexpr Vector3 CollisionSize = Vector3(0.8f, 1.8f, 0.8f);
				const auto chunks = CreateChunks({ Lattice2(4, 12), Lattice2(3, 12), Lattice2(5, 12), Lattice2(4, 12) });

				// 1チャンクの中に収まっている
				test(
					(5.0f, 4.01f, 5.0f),
					true, (5, 6), (4, 6), (5, 6),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// X方向にだけ跨っている
				test(
					(15.4f, 4.01f, 5.0f),
					true, (15, 15), (4, 6), (5, 6),
					true, (0, 0), (4, 6), (5, 6),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// Z方向にだけ跨っている
				test(
					(5.0f, 4.01f, 15.4f),
					true, (5, 6), (4, 6), (15, 15),
					false, (0, 0), (0, 0), (0, 0),
					true, (5, 6), (4, 6), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// XZ両方向に跨っている
				test(
					(15.4f, 4.01f, 15.4f),
					true, (15, 15), (4, 6), (15, 15),
					true, (0, 0), (4, 6), (15, 15),
					true, (15, 15), (4, 6), (0, 0),
					true, (0, 0), (4, 6), (0, 0)
				);

				// チャンク配列のインデックスを超えた
				test(
					(31.4f, 4.01f, 5.0f),
					true, (15, 15), (4, 6), (5, 6),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0), // 配列外・チャンクを跨ぐ ことが両立することはない
					false, (0, 0), (0, 0), (0, 0)
				);

#undef test
			}
		};

#undef eq
#undef neq
	};
}

#endif
