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
		static bool IsValidChunkIndex(const int chunkIndex, int chunkCount)
		{
			return 0 <= chunkIndex && chunkIndex < chunkCount;
		}

		// チャンクインデックスが有効であるか
		// 即ち、上下限を超えた値でないか
		static bool IsValidChunkIndex(const Lattice2& chunkIndex, int chunkCount)
		{
			return IsValidChunkIndex(chunkIndex.x, chunkCount)
				&& IsValidChunkIndex(chunkIndex.y, chunkCount);
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
		/// <para>各チャンクについて、(コリジョン立方体が属するか, ローカルチャンクインデックス, X方向の範囲, Y方向の範囲, Z方向の範囲) のタプルを返す</para>
		/// <para>ローカルチャンクインデックスは、そのチャンクが 2x2 チャンク群内で何番目かを示す (0~1 の範囲)</para>
		/// <para>範囲は、そのチャンクにおけるローカルブロック座標</para>
		/// <para>そのチャンクに属さない場合、範囲はデフォルト値</para>
		/// <para>そのチャンクがチャンク配列の範囲外の場合、属さない扱いになる</para>
		/// <para>そもそもの開始座標がチャンク配列の範囲外の場合、全てのチャンクで属さない扱いになる</para>
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

			// 開始地点のチャンクが配列の範囲外
			if (!IsValidChunkIndex(chunkIndexMin, ChunkCount))
			{
				return
				{
					std::make_tuple(false, Lattice2(0, 0), Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero()),
					std::make_tuple(false, Lattice2(1, 0), Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero()),
					std::make_tuple(false, Lattice2(0, 1), Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero()),
					std::make_tuple(false, Lattice2(1, 1), Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero()),
				};
			}
			// 以降、開始地点のチャンクには、必ずコリジョンが属する

			// 各チャンクが配列の範囲内か
			// x0z0, x1z0, x0z1, x1z1 の順番
			const bool isValidChunkFlags[4] =
			{
				true,
				IsValidChunkIndex(chunkIndexMax.x, ChunkCount),
				IsValidChunkIndex(chunkIndexMax.y, ChunkCount),
				IsValidChunkIndex(chunkIndexMax, ChunkCount)
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
					isCrossingChunkX ? Terrain::ChunkSize : 0,
					0,
					isCrossingChunkZ ? Terrain::ChunkSize : 0
				);

			std::array<std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>, 4> result = {};
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
					result[i] = std::make_tuple(false, localChunkIndex, Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero());
					continue;
				}

				// コリジョンが属さないチャンクなので、スキップ
				if (
					(i == 1 && !isCrossingChunkX) ||
					(i == 2 && !isCrossingChunkZ) ||
					(i == 3 && !(isCrossingChunkX && isCrossingChunkZ))
					)
				{
					result[i] = std::make_tuple(false, localChunkIndex, Lattice2::Zero(), Lattice2::Zero(), Lattice2::Zero());
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

				result[i] = std::make_tuple(true, localChunkIndex, rangeX, rangeY, rangeZ);
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
				[&terrainChunks, &position, &size](std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2> boundaryInfo) -> int
				{
					// 情報をアンパック
					const auto& [isInsideChunk, localChunkIndex, rangeX, rangeY, rangeZ] = boundaryInfo;

					// このチャンクにコリジョンが属していないので、最小値を返す
					if (!isInsideChunk)
						return -1;

					// チャンクを取得 (配列の範囲外なら、最小値を返す)
					const Lattice2 chunkIndex = GetChunkIndex(GetBlockPosition(GetCollisionMinPosition(position, size))) + localChunkIndex;
					if (!IsValidChunkIndex(chunkIndex, ChunkCount))
						return -1;
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
				[&terrainChunks, &position, &size](std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2> boundaryInfo) -> int
				{
					// 情報をアンパック
					const auto& [isInsideChunk, localChunkIndex, rangeX, rangeY, rangeZ] = boundaryInfo;

					// このチャンクにコリジョンが属していないので、最大値を返す
					if (!isInsideChunk)
						return Terrain::ChunkHeight;

					// チャンクを取得 (配列の範囲外なら、最大値を返す)
					const Lattice2 chunkIndex = GetChunkIndex(GetBlockPosition(GetCollisionMinPosition(position, size))) + localChunkIndex;
					if (!IsValidChunkIndex(chunkIndex, ChunkCount))
						return Terrain::ChunkHeight;
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
				[&terrainChunks, &position, &size](std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2> boundaryInfo) -> bool
				{
					// 情報をアンパック
					const auto& [isInsideChunk, localChunkIndex, rangeX, rangeY, rangeZ] = boundaryInfo;

					// このチャンクにコリジョンが属していないので、重なっていない
					if (!isInsideChunk)
						return false;

					// チャンクを取得 (配列の範囲外なら、重なっていないとみなす)
					const Lattice2 chunkIndex = GetChunkIndex(GetBlockPosition(GetCollisionMinPosition(position, size))) + localChunkIndex;
					if (!IsValidChunkIndex(chunkIndex, ChunkCount))
						return false;
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
				for (int i = 0; i < 4; ++i)
				{
					const auto& [l0, l1, l2, l3, l4] = value[i];
					const auto& [r0, r1, r2, r3, r4] = other.value[i];

					if (l0 != r0) return false;
					if (l1 != r1) return false;
					if (l2 != r2) return false;
					if (l3 != r3) return false;
					if (l4 != r4) return false;
				}

				return true;
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

		// std::string のコンストラクタに渡す
#define eqs(value, expected) { if ((value) != (expected)) ForiverEngine::Test_PlayerControl::Assert((std::string(value)), (std::string(expected)), __FILE__, __LINE__); }
#define neqs(value, expected) { if ((value) == (expected)) ForiverEngine::Test_PlayerControl::Assert((std::string(value)), (std::string(expected)), __FILE__, __LINE__); }

		// 列挙型 (static_cast<int> する)
#define eqen(value, expected) { if ((value) != (expected)) ForiverEngine::Test_PlayerControl::Assert((static_cast<int>(value)), (static_cast<int>(expected)), __FILE__, __LINE__); }
#define neqen(value, expected) { if ((value) == (expected)) ForiverEngine::Test_PlayerControl::Assert((static_cast<int>(value)), (static_cast<int>(expected)), __FILE__, __LINE__); }

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
				Run_IsValidChunkIndex1();
				Run_IsValidChunkIndex2();
				Run_GetFootPosition();
				Run_GetCollisionMinPosition();

				Run_Rotate();
				Run_MoveH();

				Run_CreateChunks2x2();
				Run_CalculateCollisionBoundaryAsBlock();
				Run_FindFloorHeight();
				Run_FindCeilHeight();
				Run_IsOverlappingWithTerrain();
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

			static void Run_IsValidChunkIndex1()
			{
				eq(PlayerControl::IsValidChunkIndex(0, 4), true);
				eq(PlayerControl::IsValidChunkIndex(3, 4), true);
				eq(PlayerControl::IsValidChunkIndex(-1, 4), false);
				eq(PlayerControl::IsValidChunkIndex(4, 4), false);
			}

			static void Run_IsValidChunkIndex2()
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

#pragma region Complex Terrain Calculations

			static constexpr Vector3 PlayerCollisionSize = Vector3(0.8f, 1.8f, 0.8f);

			static std::array<std::array<Terrain, 2>, 2> CreateChunks2x2()
			{
				static const auto chunks2x2 = CreateChunks({ Lattice2(4, 12), Lattice2(3, 13), Lattice2(5, 11), Lattice2(4, 12) });
				return chunks2x2;
			}

			static void Run_CreateChunks2x2()
			{
				const auto chunks2x2 = CreateChunks2x2();

				// x0z0
				eqen(chunks2x2[0][0].GetBlock(0, 3, 0), Block::Stone);
				eqen(chunks2x2[0][0].GetBlock(0, 4, 0), Block::Air);
				eqen(chunks2x2[0][0].GetBlock(0, 12, 0), Block::Air);
				eqen(chunks2x2[0][0].GetBlock(0, 13, 0), Block::Stone);
				eqen(chunks2x2[0][0].GetBlock(12, 1, 15), Block::Stone);
				eqen(chunks2x2[0][0].GetBlock(12, 8, 15), Block::Air);
				eqen(chunks2x2[0][0].GetBlock(12, 64, 15), Block::Stone);

				// x1z0
				eqen(chunks2x2[1][0].GetBlock(0, 2, 0), Block::Stone);
				eqen(chunks2x2[1][0].GetBlock(0, 3, 0), Block::Air);
				eqen(chunks2x2[1][0].GetBlock(0, 13, 0), Block::Air);
				eqen(chunks2x2[1][0].GetBlock(0, 14, 0), Block::Stone);
				eqen(chunks2x2[1][0].GetBlock(12, 1, 15), Block::Stone);
				eqen(chunks2x2[1][0].GetBlock(12, 8, 15), Block::Air);
				eqen(chunks2x2[1][0].GetBlock(12, 64, 15), Block::Stone);

				// x0z1
				eqen(chunks2x2[0][1].GetBlock(0, 4, 0), Block::Stone);
				eqen(chunks2x2[0][1].GetBlock(0, 5, 0), Block::Air);
				eqen(chunks2x2[0][1].GetBlock(0, 11, 0), Block::Air);
				eqen(chunks2x2[0][1].GetBlock(0, 12, 0), Block::Stone);
				eqen(chunks2x2[0][1].GetBlock(12, 1, 15), Block::Stone);
				eqen(chunks2x2[0][1].GetBlock(12, 8, 15), Block::Air);
				eqen(chunks2x2[0][1].GetBlock(12, 64, 15), Block::Stone);

				// x1z1
				eqen(chunks2x2[1][1].GetBlock(0, 3, 0), Block::Stone);
				eqen(chunks2x2[1][1].GetBlock(0, 4, 0), Block::Air);
				eqen(chunks2x2[1][1].GetBlock(0, 12, 0), Block::Air);
				eqen(chunks2x2[1][1].GetBlock(0, 13, 0), Block::Stone);
				eqen(chunks2x2[1][1].GetBlock(12, 1, 15), Block::Stone);
				eqen(chunks2x2[1][1].GetBlock(12, 8, 15), Block::Air);
				eqen(chunks2x2[1][1].GetBlock(12, 64, 15), Block::Stone);
			}

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
		CreateChunks2x2(), \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	)), CollisionBoundaryInfoWrapper(std::array<std::tuple<bool, Lattice2, Lattice2, Lattice2, Lattice2>, 4> \
	{ \
		std::make_tuple(##b00, Lattice2(0, 0), Lattice2##x00, Lattice2##y00, Lattice2##z00), \
		std::make_tuple(##b10, Lattice2(1, 0), Lattice2##x10, Lattice2##y10, Lattice2##z10), \
		std::make_tuple(##b01, Lattice2(0, 1), Lattice2##x01, Lattice2##y01, Lattice2##z01), \
		std::make_tuple(##b11, Lattice2(1, 1), Lattice2##x11, Lattice2##y11, Lattice2##z11), \
	})); \
} \

				// 1チャンクの中に収まっている
				test(
					(5.0f, 4.0f, 5.0f),
					true, (5, 6), (4, 6), (5, 6),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// X方向にだけ跨っている
				test(
					(15.4f, 4.0f, 5.0f),
					true, (15, 15), (4, 6), (5, 6),
					true, (0, 0), (4, 6), (5, 6),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// Z方向にだけ跨っている
				test(
					(5.0f, 4.0f, 15.4f),
					true, (5, 6), (4, 6), (15, 15),
					false, (0, 0), (0, 0), (0, 0),
					true, (5, 6), (4, 6), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// XZ両方向に跨っている
				test(
					(15.4f, 4.0f, 15.4f),
					true, (15, 15), (4, 6), (15, 15),
					true, (0, 0), (4, 6), (15, 15),
					true, (15, 15), (4, 6), (0, 0),
					true, (0, 0), (4, 6), (0, 0)
				);

				// チャンク配列の範囲外 (最小座標は範囲内)
				test(
					(31.4f, 4.0f, 15.4f),
					true, (15, 15), (4, 6), (15, 15),
					false, (0, 0), (0, 0), (0, 0),
					true, (15, 15), (4, 6), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// チャンク配列の範囲外 (最小座標が範囲外)
				test(
					(32.1f, 4.0f, 15.4f),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

#undef test
			}

			static void Run_FindFloorHeight()
			{
#define test(worldPositionMin, expected)  \
{ \
	eq(PlayerControl::FindFloorHeight( \
		CreateChunks2x2(), \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	), expected); \
} \

				// 足元が埋まっている
				test((5.0f, 2.0f, 5.0f), 1);

				// 1チャンクの中に収まっている
				// x0z0, x1z0, x0z1, x1z1
				test((5.0f, 8.0f, 5.0f), 3);
				test((24.0f, 8.0f, 5.0f), 2);
				test((5.0f, 8.0f, 24.0f), 4);
				test((24.0f, 8.0f, 24.0f), 3);

				// 複数チャンクに跨っている
				// xのみ, zのみ, xz両方
				test((15.4f, 8.0f, 5.0f), 3); // x0z0, x1z0 で判定
				test((5.0f, 8.0f, 15.4f), 4); // x0z0, x0z1 で判定
				test((15.4f, 8.0f, 15.4f), 4); // x0z0, x1z0, x0z1, x1z1 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				// 1チャンクの中に収まっている
				// xのみ範囲外, zのみ範囲外, xz両方範囲外
				test((31.4f, 8.0f, 5.0f), 2); // x1z0 で判定
				test((5.0f, 8.0f, 31.4f), 4); // x0z1 で判定
				test((31.4f, 8.0f, 31.4f), 3); // x1z1 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				// 複数チャンクに跨っている
				// x範囲外z跨っている, z範囲外x跨っている, xz両方範囲外
				test((31.4f, 8.0f, 15.4f), 3); // x1z0, x1z1 で判定
				test((15.4f, 8.0f, 31.4f), 4); // x0z1, x1z1 で判定

				// チャンク配列の範囲外 (最小座標が範囲外)
				test((32.1f, 8.0f, 15.4f), -1);

#undef test
			}

			static void Run_FindCeilHeight()
			{
#define test(worldPositionMin, expected)  \
{ \
	eq(PlayerControl::FindCeilHeight( \
		CreateChunks2x2(), \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	), expected); \
} \

				// 頭上が埋まっている
				test((5.0f, 24.0f, 5.0f), 27);

				// 1チャンクの中に収まっている
				// x0z0, x1z0, x0z1, x1z1
				test((5.0f, 8.0f, 5.0f), 13);
				test((24.0f, 8.0f, 5.0f), 14);
				test((5.0f, 8.0f, 24.0f), 12);
				test((24.0f, 8.0f, 24.0f), 13);

				// 複数チャンクに跨っている
				// xのみ, zのみ, xz両方
				test((15.4f, 8.0f, 5.0f), 13); // x0z0, x1z0 で判定
				test((5.0f, 8.0f, 15.4f), 12); // x0z0, x0z1 で判定
				test((15.4f, 8.0f, 15.4f), 12); // x0z0, x1z0, x0z1, x1z1 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				// 1チャンクの中に収まっている
				// xのみ範囲外, zのみ範囲外, xz両方範囲外
				test((31.4f, 8.0f, 5.0f), 14); // x1z0 で判定
				test((5.0f, 8.0f, 31.4f), 12); // x0z1 で判定
				test((31.4f, 8.0f, 31.4f), 13); // x1z1 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				// 複数チャンクに跨っている
				// x範囲外z跨っている, z範囲外x跨っている, xz両方範囲外
				test((31.4f, 8.0f, 15.4f), 13); // x1z0, x1z1 で判定
				test((15.4f, 8.0f, 31.4f), 12); // x0z1, x1z1 で判定

				// チャンク配列の範囲外 (最小座標が範囲外)
				test((32.1f, 8.0f, 15.4f), Terrain::ChunkHeight);

#undef test
			}

			static void Run_IsOverlappingWithTerrain()
			{
#define test(worldPositionMin, expected)  \
{ \
	eq(PlayerControl::IsOverlappingWithTerrain( \
		CreateChunks2x2(), \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	), expected); \
} \

				// 足元
				test((5.0f, 3.49f, 5.0f), true); // x0z0 で判定
				test((5.0f, 3.51f, 5.0f), false); // x0z0 で判定

				// 頭上
				test((5.0f, 10.69f, 5.0f), false); // x0z0 で判定
				test((5.0f, 10.71f, 5.0f), true); // x0z0 で判定

				// 横
				test((15.4f, 2.49f, 5.0f), true); // x0z0, x1z0 で判定
				test((15.4f, 2.51f, 5.0f), true); // x0z0, x1z0 で判定
				test((15.4f, 3.49f, 5.0f), true); // x0z0, x1z0 で判定
				test((15.4f, 3.51f, 5.0f), false); // x0z0, x1z0 で判定

				// ブロック座標の境界ギリギリ
				test((5.0f, 3.49f, 5.0f), true); // x0z0 で判定
				test((5.0f, 3.50f, 5.0f), false); // x0z0 で判定
				test((5.0f, 3.51f, 5.0f), false); // x0z0 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				test((31.4f, 2.49f, 5.0f), true); // x1z0 で判定
				test((31.4f, 2.51f, 5.0f), false); // x1z0 で判定
				test((31.4f, 2.49f, 15.4f), true); // x1z0, x1z1 で判定
				test((31.4f, 2.51f, 15.4f), true); // x1z0, x1z1 で判定
				test((31.4f, 3.49f, 15.4f), true); // x1z0, x1z1 で判定
				test((31.4f, 3.51f, 15.4f), false); // x1z0, x1z1 で判定

				// チャンク配列の範囲外 (最小座標が範囲外)
				test((32.1f, 2.0f, 15.4f), false);
				test((32.1f, 8.0f, 15.4f), false);

#undef test
			}

#pragma endregion
		};

#undef eq
#undef neq
	};
}

#endif
