#pragma once

#include <scripts/test/IncludeInternal.h>

namespace ForiverEngine
{
	namespace Test
	{
		struct PlayerControl final
		{
		public:
			DELETE_DEFAULT_METHODS(PlayerControl);

			static void RunAll()
			{
				Run_GetBlockPosition_1();
				Run_GetBlockPosition_3();
				Run_GetFootPosition();
				Run_GetCollisionMinPosition();

				Run_CreateChunksManager3Layered2x2ForTest();
				Run_CalculateCollisionBoundaryAsBlock();
				Run_FindFloorHeight();
				Run_FindCeilHeight();
				Run_IsOverlappingWithTerrain();
				Run_IsOverlappingWithBlock();
			}

			using TargetClass = ForiverEngine::PlayerControl;

#pragma region Primitive Calculations

			static void Run_GetBlockPosition_1()
			{
				eq(TargetClass::GetBlockPosition(0.0f), 0);
				eq(TargetClass::GetBlockPosition(0.4f), 0);
				eq(TargetClass::GetBlockPosition(0.5f), 1);
				eq(TargetClass::GetBlockPosition(0.6f), 1);
				eq(TargetClass::GetBlockPosition(1.0f), 1);
				eq(TargetClass::GetBlockPosition(-0.4f), 0);
				eq(TargetClass::GetBlockPosition(-0.5f), -1);
			}

			static void Run_GetBlockPosition_3()
			{
				eqla(TargetClass::GetBlockPosition(Vector3(0.0f, 1.0f, 2.0f)), Lattice3(0, 1, 2));
			}

			static void Run_GetFootPosition()
			{
				eqla(TargetClass::GetFootPosition(Vector3(0.0f, 1.8f, 0.0f), 1.8f), Vector3(0.0f, 0.0f, 0.0f));
				eqla(TargetClass::GetFootPosition(Vector3(1.0f, 2.5f, -3.0f), 1.5f), Vector3(1.0f, 1.0f, -3.0f));
			}

			static void Run_GetCollisionMinPosition()
			{
				eqla(TargetClass::GetCollisionMinPosition(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.6f, 1.8f, 0.6f)), Vector3(-0.3f, 0.0f, -0.3f));
				eqla(TargetClass::GetCollisionMinPosition(Vector3(1.0f, 2.0f, 3.0f), Vector3(1.0f, 2.0f, 1.0f)), Vector3(0.5f, 2.0f, 2.5f));
			}

#pragma endregion

#pragma region Complex Chunk Calculations

#pragma region Chunk Helpers

			struct CollisionBoundaryInfoWrapper
			{
				std::array<TargetClass::CollisionBoundaryAsBlockInfoPerChunk, 4> value{};

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
						text += std::format(
							"[{},{},{},{}],",
							info.isContained ? "o" : "x",
							(info.isContained || info.rangeX != Lattice2::Zero()) ? ForiverEngine::ToString(info.rangeX) : "_",
							(info.isContained || info.rangeY != Lattice2::Zero()) ? ForiverEngine::ToString(info.rangeY) : "_",
							(info.isContained || info.rangeZ != Lattice2::Zero()) ? ForiverEngine::ToString(info.rangeZ) : "_"
						);
					}
					text += "}";

					return text;
				}
			};

			// チャンクを手動作成
			// y[0, Terrain::ChunkHeight-1] の各層が存在するかどうかを layers で指定
			static Chunk CreateChunkLayerd(const std::array<bool, Chunk::Height>& layers)
			{
				Chunk chunk = Chunk::CreateVoid();
				for (int y = 0; y < Chunk::Height; ++y)
				{
					if (layers[y])
					{
						for (int x = 0; x < Chunk::Size; ++x)
							for (int z = 0; z < Chunk::Size; ++z)
							{
								chunk.SetBlock({ x, y, z }, Block::Stone);
							}
					}
				}

				return chunk;
			}

			// ブロック、空気、ブロックの 3層構造のチャンクを作成
			static Chunk CreateChunk3Layerd(const Lattice2& airYRange)
			{
				std::array<bool, Chunk::Height> layers = {};
				for (int y = 0; y < Chunk::Height; ++y)
				{
					if (!MathUtils::IsInRange(y, airYRange.x, airYRange.y + 1))
						layers[y] = true;
				}

				return CreateChunkLayerd(layers);
			}

			// Read
			// 強引にチャンクデータを引き抜く
			static const Chunk& ExstractChunk(const ChunksManager& chunksManager, const Lattice2& chunkIndex)
			{
				class ChunksManager_Dummy
				{
				public:
					Chunk::ChunksArray<std::atomic<std::uint8_t>> generationStates;
					Chunk::ChunksArray<Chunk> chunks;
				};

				const ChunksManager_Dummy* chunksManager_Dummy = reinterpret_cast<const ChunksManager_Dummy*>(&chunksManager);
				return chunksManager_Dummy->chunks[chunkIndex.x][chunkIndex.y];
			}

			// Write
			// 強引にチャンクデータを上書きする
			static void OverwriteChunk(const ChunksManager& chunksManager, const Lattice2& chunkIndex, Chunk&& newChunk)
			{
				class ChunksManager_Dummy
				{
				public:
					Chunk::ChunksArray<std::atomic<std::uint8_t>> generationStates;
					Chunk::ChunksArray<Chunk> chunks;
				};

				const ChunksManager_Dummy* chunksManager_Dummy = reinterpret_cast<const ChunksManager_Dummy*>(&chunksManager);
				chunksManager_Dummy->chunks[chunkIndex.x][chunkIndex.y] = std::move(newChunk);
			}

			// チャンク群を作成し、以下のチャンクについてデータを設定する (= 引数の配列における順番)
			// [0,0], [1,0], [0,1], [1,1],
			// [-2,-2], [-1,-2], [-2,-1], [-1,-1]
			// ブロック、空気、ブロックの 3層構造
			// min, x隣, z隣, xz隣 の順に airYRanges を指定
			// データを設定したチャンクの隣接チャンクは空にしておく
			static ChunksManager CreateChunksManager3Layerd2x2WithDiagonal(const std::array<Lattice2, 8>& airYRanges)
			{
				ChunksManager chunksManager = ChunksManager(Lattice2::Zero());

				OverwriteChunk(chunksManager, { 0, 0 }, CreateChunk3Layerd(airYRanges[0]));
				OverwriteChunk(chunksManager, { 1, 0 }, CreateChunk3Layerd(airYRanges[1]));
				OverwriteChunk(chunksManager, { 0, 1 }, CreateChunk3Layerd(airYRanges[2]));
				OverwriteChunk(chunksManager, { 1, 1 }, CreateChunk3Layerd(airYRanges[3]));
				OverwriteChunk(chunksManager, { Chunk::Count - 2, Chunk::Count - 2 }, CreateChunk3Layerd(airYRanges[4]));
				OverwriteChunk(chunksManager, { Chunk::Count - 1, Chunk::Count - 2 }, CreateChunk3Layerd(airYRanges[5]));
				OverwriteChunk(chunksManager, { Chunk::Count - 2, Chunk::Count - 1 }, CreateChunk3Layerd(airYRanges[6]));
				OverwriteChunk(chunksManager, { Chunk::Count - 1, Chunk::Count - 1 }, CreateChunk3Layerd(airYRanges[7]));

				// 隣接チャンクは空にしておく
				OverwriteChunk(chunksManager, { 0, 2 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { 1, 2 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { 2, 0 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { 2, 1 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { 2, 2 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { Chunk::Count - 1, Chunk::Count - 3 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { Chunk::Count - 2, Chunk::Count - 3 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { Chunk::Count - 3, Chunk::Count - 1 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { Chunk::Count - 3, Chunk::Count - 2 }, Chunk::CreateVoid());
				OverwriteChunk(chunksManager, { Chunk::Count - 3, Chunk::Count - 3 }, Chunk::CreateVoid());

				return chunksManager;
			}

#pragma endregion

			static constexpr Vector3 PlayerCollisionSize = Vector3(0.8f, 1.8f, 0.8f);

			static const ChunksManager& CreateChunksManager3Layered2x2ForTest()
			{
				static bool hasCreated = false;
				static ChunksManager chunksManager;

				if (!hasCreated)
				{
					hasCreated = true;
					chunksManager = CreateChunksManager3Layerd2x2WithDiagonal({
						Lattice2(4, 12), Lattice2(3, 13), Lattice2(5, 11), Lattice2(4, 12),
						Lattice2(4, 12), Lattice2(3, 13), Lattice2(5, 11), Lattice2(4, 12),
						});
				}

				return chunksManager;
			}

			static void Run_CreateChunksManager3Layered2x2ForTest()
			{
				const ChunksManager& chunksManager = CreateChunksManager3Layered2x2ForTest();

#define test(chunkIndex, blockPosition, expectedBlock) \
eqen(ExstractChunk(chunksManager, Lattice2##chunkIndex).GetBlock(Lattice3##blockPosition), Block::##expectedBlock); \

				// x0z0
				test((0, 0), (0, 3, 0), Stone);
				test((0, 0), (0, 4, 0), Air);
				test((0, 0), (0, 12, 0), Air);
				test((0, 0), (0, 13, 0), Stone);
				test((0, 0), (12, 1, 15), Stone);
				test((0, 0), (12, 8, 15), Air);
				test((0, 0), (12, 64, 15), Stone);

				// x1z0
				test((1, 0), (0, 2, 0), Stone);
				test((1, 0), (0, 3, 0), Air);
				test((1, 0), (0, 13, 0), Air);
				test((1, 0), (0, 14, 0), Stone);
				test((1, 0), (12, 1, 15), Stone);
				test((1, 0), (12, 8, 15), Air);
				test((1, 0), (12, 64, 15), Stone);

				// x0z1
				test((0, 1), (0, 4, 0), Stone);
				test((0, 1), (0, 5, 0), Air);
				test((0, 1), (0, 11, 0), Air);
				test((0, 1), (0, 12, 0), Stone);
				test((0, 1), (12, 1, 15), Stone);
				test((0, 1), (12, 8, 15), Air);
				test((0, 1), (12, 64, 15), Stone);

				// x1z1
				test((1, 1), (0, 3, 0), Stone);
				test((1, 1), (0, 4, 0), Air);
				test((1, 1), (0, 12, 0), Air);
				test((1, 1), (0, 13, 0), Stone);
				test((1, 1), (12, 1, 15), Stone);
				test((1, 1), (12, 8, 15), Air);
				test((1, 1), (12, 64, 15), Stone);

				// x-2z-2
				test((Chunk::Count - 2, Chunk::Count - 2), (0, 3, 0), Stone);
				test((Chunk::Count - 2, Chunk::Count - 2), (0, 4, 0), Air);
				test((Chunk::Count - 2, Chunk::Count - 2), (0, 12, 0), Air);
				test((Chunk::Count - 2, Chunk::Count - 2), (0, 13, 0), Stone);
				test((Chunk::Count - 2, Chunk::Count - 2), (12, 1, 15), Stone);
				test((Chunk::Count - 2, Chunk::Count - 2), (12, 8, 15), Air);
				test((Chunk::Count - 2, Chunk::Count - 2), (12, 64, 15), Stone);

				// x-1z-2
				test((Chunk::Count - 1, Chunk::Count - 2), (0, 2, 0), Stone);
				test((Chunk::Count - 1, Chunk::Count - 2), (0, 3, 0), Air);
				test((Chunk::Count - 1, Chunk::Count - 2), (0, 13, 0), Air);
				test((Chunk::Count - 1, Chunk::Count - 2), (0, 14, 0), Stone);
				test((Chunk::Count - 1, Chunk::Count - 2), (12, 1, 15), Stone);
				test((Chunk::Count - 1, Chunk::Count - 2), (12, 8, 15), Air);
				test((Chunk::Count - 1, Chunk::Count - 2), (12, 64, 15), Stone);

				// x-2z-1
				test((Chunk::Count - 2, Chunk::Count - 1), (0, 4, 0), Stone);
				test((Chunk::Count - 2, Chunk::Count - 1), (0, 5, 0), Air);
				test((Chunk::Count - 2, Chunk::Count - 1), (0, 11, 0), Air);
				test((Chunk::Count - 2, Chunk::Count - 1), (0, 12, 0), Stone);
				test((Chunk::Count - 2, Chunk::Count - 1), (12, 1, 15), Stone);
				test((Chunk::Count - 2, Chunk::Count - 1), (12, 8, 15), Air);
				test((Chunk::Count - 2, Chunk::Count - 1), (12, 64, 15), Stone);

				// x-1z-1
				test((Chunk::Count - 1, Chunk::Count - 1), (0, 3, 0), Stone);
				test((Chunk::Count - 1, Chunk::Count - 1), (0, 4, 0), Air);
				test((Chunk::Count - 1, Chunk::Count - 1), (0, 12, 0), Air);
				test((Chunk::Count - 1, Chunk::Count - 1), (0, 13, 0), Stone);
				test((Chunk::Count - 1, Chunk::Count - 1), (12, 1, 15), Stone);
				test((Chunk::Count - 1, Chunk::Count - 1), (12, 8, 15), Air);
				test((Chunk::Count - 1, Chunk::Count - 1), (12, 64, 15), Stone);

#undef test
			}

			static void Run_CalculateCollisionBoundaryAsBlock()
			{
#define test( \
	worldPositionMin, \
	b00, x00, y00, z00, \
	b10, x10, y10, z10, \
	b01, x01, y01, z01, \
	b11, x11, y11, z11 \
) \
eqobj( \
	(CollisionBoundaryInfoWrapper{TargetClass::CalculateCollisionBoundaryAsBlock( \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	)}), \
	(CollisionBoundaryInfoWrapper{{ \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b00, Lattice2(0, 0), Lattice2##x00, Lattice2##y00, Lattice2##z00 }, \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b10, Lattice2(1, 0), Lattice2##x10, Lattice2##y10, Lattice2##z10 }, \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b01, Lattice2(0, 1), Lattice2##x01, Lattice2##y01, Lattice2##z01 }, \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b11, Lattice2(1, 1), Lattice2##x11, Lattice2##y11, Lattice2##z11 }, \
	}}) \
); \

#define test_diag( \
	worldPositionMin, \
	b00, x00, y00, z00, \
	b10, x10, y10, z10, \
	b01, x01, y01, z01, \
	b11, x11, y11, z11 \
) \
eqobj( \
	(CollisionBoundaryInfoWrapper{TargetClass::CalculateCollisionBoundaryAsBlock( \
		Vector3##worldPositionMin + Vector3((Chunk::Count - 2) * Chunk::Size, 0, (Chunk::Count - 2) * Chunk::Size), \
		PlayerCollisionSize \
	)}), \
	(CollisionBoundaryInfoWrapper{{ \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b00, Lattice2(0, 0), Lattice2##x00, Lattice2##y00, Lattice2##z00 }, \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b10, Lattice2(1, 0), Lattice2##x10, Lattice2##y10, Lattice2##z10 }, \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b01, Lattice2(0, 1), Lattice2##x01, Lattice2##y01, Lattice2##z01 }, \
		TargetClass::CollisionBoundaryAsBlockInfoPerChunk { ##b11, Lattice2(1, 1), Lattice2##x11, Lattice2##y11, Lattice2##z11 }, \
	}}) \
); \

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

				// チャンク配列の範囲外 (最小座標は範囲内 XZ)
				test_diag(
					(31.4f, 4.0f, 15.4f),
					true, (15, 15), (4, 6), (15, 15),
					false, (0, 0), (0, 0), (0, 0),
					true, (15, 15), (4, 6), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// チャンク配列の範囲外 (最小座標は範囲内 Y)
				test(
					(15.4f, Chunk::Height - 0.6f, 15.4f),
					true, (15, 15), (Chunk::Height - 1, Chunk::Height - 1), (15, 15),
					true, (0, 0), (Chunk::Height - 1, Chunk::Height - 1), (15, 15),
					true, (15, 15), (Chunk::Height - 1, Chunk::Height - 1), (0, 0),
					true, (0, 0), (Chunk::Height - 1, Chunk::Height - 1), (0, 0)
				);

				// チャンク配列の範囲外 (最小座標が範囲外 XZ)
				test(
					(-1.0f, 4.0f, -1.0f),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

				// チャンク配列の範囲外 (最小座標が範囲外 Y)
				test(
					(15.4f, -1.0f, 15.4f),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);
				test(
					(15.4f, 1.0f * Chunk::Height, 15.4f),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0),
					false, (0, 0), (0, 0), (0, 0)
				);

#undef test
#undef test_diag
			}

			static void Run_FindFloorHeight()
			{
#define test(worldPositionMin, expected) \
eq( \
	(TargetClass::FindFloorHeight( \
		CreateChunksManager3Layered2x2ForTest().GetChunks(), \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	)), \
	(expected) \
); \

#define test_diag(worldPositionMin, expected) \
eq( \
	(TargetClass::FindFloorHeight( \
		CreateChunksManager3Layered2x2ForTest().GetChunks(), \
		Vector3##worldPositionMin + Vector3((Chunk::Count - 2) * Chunk::Size, 0, (Chunk::Count - 2) * Chunk::Size), \
		PlayerCollisionSize \
	)), \
	(expected) \
); \

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
				test_diag((31.4f, 8.0f, 5.0f), 2); // x1z0 で判定
				test_diag((5.0f, 8.0f, 31.4f), 4); // x0z1 で判定
				test_diag((31.4f, 8.0f, 31.4f), 3); // x1z1 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				// 複数チャンクに跨っている
				// x範囲外z跨っている, z範囲外x跨っている, xz両方範囲外
				test_diag((31.4f, 8.0f, 15.4f), 3); // x1z0, x1z1 で判定
				test_diag((15.4f, 8.0f, 31.4f), 4); // x0z1, x1z1 で判定

				// チャンク配列の範囲外 (最小座標が範囲外)
				test((-1.0f, 8.0f, -1.0f), -1);

#undef test
#undef test_diag
			}

			static void Run_FindCeilHeight()
			{
#define test(worldPositionMin, expected) \
eq( \
	(TargetClass::FindCeilHeight( \
		CreateChunksManager3Layered2x2ForTest().GetChunks(), \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	)), \
	(expected) \
); \

#define test_diag(worldPositionMin, expected) \
eq( \
	(TargetClass::FindCeilHeight( \
		CreateChunksManager3Layered2x2ForTest().GetChunks(), \
		Vector3##worldPositionMin + Vector3((Chunk::Count - 2) * Chunk::Size, 0, (Chunk::Count - 2) * Chunk::Size), \
		PlayerCollisionSize \
	)), \
	(expected) \
); \

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
				test_diag((31.4f, 8.0f, 5.0f), 14); // x1z0 で判定
				test_diag((5.0f, 8.0f, 31.4f), 12); // x0z1 で判定
				test_diag((31.4f, 8.0f, 31.4f), 13); // x1z1 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				// 複数チャンクに跨っている
				// x範囲外z跨っている, z範囲外x跨っている, xz両方範囲外
				test_diag((31.4f, 8.0f, 15.4f), 13); // x1z0, x1z1 で判定
				test_diag((15.4f, 8.0f, 31.4f), 12); // x0z1, x1z1 で判定

				// チャンク配列の範囲外 (最小座標が範囲外)
				test((-1.0f, 8.0f, -1.0f), Chunk::Height);

#undef test
#undef test_diag
			}

			static void Run_IsOverlappingWithTerrain()
			{
#define test(worldPositionMin, expected) \
eq( \
	(TargetClass::IsOverlappingWithTerrain( \
		CreateChunksManager3Layered2x2ForTest().GetChunks(), \
		Vector3##worldPositionMin, \
		PlayerCollisionSize \
	)), \
	(expected) \
); \

#define test_diag(worldPositionMin, expected) \
eq( \
	(TargetClass::IsOverlappingWithTerrain( \
		CreateChunksManager3Layered2x2ForTest().GetChunks(), \
		Vector3##worldPositionMin + Vector3((Chunk::Count - 2) * Chunk::Size, 0, (Chunk::Count - 2) * Chunk::Size), \
		PlayerCollisionSize \
	)), \
	(expected) \
); \

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
				test_diag((31.4f, 2.49f, 5.0f), true); // x1z0 で判定
				test_diag((31.4f, 2.51f, 5.0f), false); // x1z0 で判定
				test_diag((31.4f, 2.49f, 15.4f), true); // x1z0, x1z1 で判定
				test_diag((31.4f, 2.51f, 15.4f), true); // x1z0, x1z1 で判定
				test_diag((31.4f, 3.49f, 15.4f), true); // x1z0, x1z1 で判定
				test_diag((31.4f, 3.51f, 15.4f), false); // x1z0, x1z1 で判定

				// チャンク配列の範囲外 (最小座標が範囲外)
				test((-1.0f, 2.0f, -1.0f), false);
				test((-1.0f, 8.0f, -1.0f), false);

#undef test
#undef test_diag
			}

			static void Run_IsOverlappingWithBlock()
			{
#define test(footWorldPosition, targetWorldBlockPosition, expected) \
eq( \
	(TargetClass::IsOverlappingWithBlock( \
		CreateChunksManager3Layered2x2ForTest().GetChunks(), \
		Vector3##footWorldPosition, \
		PlayerCollisionSize, \
		Lattice3##targetWorldBlockPosition \
	)), \
	(expected) \
); \

				// X
				test((4.09f, 3.0f, 5.0f), (5, 3, 5), false);
				test((4.11f, 3.0f, 5.0f), (5, 3, 5), true);

				// Y
				test((5.0f, 3.51f, 5.0f), (5, 3, 5), false);
				test((5.0f, 3.49f, 5.0f), (5, 3, 5), true);

				// Z
				test((5.0f, 3.0f, 5.91f), (5, 3, 5), false);
				test((5.0f, 3.0f, 5.89f), (5, 3, 5), true);

#undef test
			}

#pragma endregion
		};
	}
}
