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
				Run_GetChunkIndex();
				Run_GetChunkLocalPosition();
				Run_IsValidChunkIndex();
				Run_GetFootPosition();
				Run_GetCollisionMinPosition();

				Run_Rotate();
				Run_MoveH();

				Run_CreateChunk3Layerd2x2ForTest();
				Run_CalculateCollisionBoundaryAsBlock();
				Run_FindFloorHeight();
				Run_FindCeilHeight();
				Run_IsOverlappingWithTerrain();
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

			static void Run_GetChunkIndex()
			{
				eqla(TargetClass::GetChunkIndex(Lattice3(0, 0, 0)), Lattice2(0, 0));
				eqla(TargetClass::GetChunkIndex(Lattice3(15, 0, 15)), Lattice2(0, 0));
				eqla(TargetClass::GetChunkIndex(Lattice3(16, 0, 0)), Lattice2(1, 0));
				eqla(TargetClass::GetChunkIndex(Lattice3(0, 0, 16)), Lattice2(0, 1));
				eqla(TargetClass::GetChunkIndex(Lattice3(16, 0, 16)), Lattice2(1, 1));
			}

			static void Run_GetChunkLocalPosition()
			{
				eqla(TargetClass::GetChunkLocalPosition(Lattice3(0, 0, 0)), Lattice3(0, 0, 0));
				eqla(TargetClass::GetChunkLocalPosition(Lattice3(15, 0, 15)), Lattice3(15, 0, 15));
				eqla(TargetClass::GetChunkLocalPosition(Lattice3(16, 0, 0)), Lattice3(0, 0, 0));
				eqla(TargetClass::GetChunkLocalPosition(Lattice3(17, 0, 0)), Lattice3(1, 0, 0));
				eqla(TargetClass::GetChunkLocalPosition(Lattice3(0, 0, 16)), Lattice3(0, 0, 0));
				eqla(TargetClass::GetChunkLocalPosition(Lattice3(0, 0, 17)), Lattice3(0, 0, 1));
				eqla(TargetClass::GetChunkLocalPosition(Lattice3(16, 0, 16)), Lattice3(0, 0, 0));
			}

			static void Run_IsValidChunkIndex()
			{
				eq(TargetClass::IsValidChunkIndex(Lattice2(0, 0), 4), true);
				eq(TargetClass::IsValidChunkIndex(Lattice2(3, 3), 4), true);
				eq(TargetClass::IsValidChunkIndex(Lattice2(-1, 0), 4), false);
				eq(TargetClass::IsValidChunkIndex(Lattice2(0, -1), 4), false);
				eq(TargetClass::IsValidChunkIndex(Lattice2(4, 0), 4), false);
				eq(TargetClass::IsValidChunkIndex(Lattice2(0, 4), 4), false);
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

#pragma region Player Movements

			static void Run_Rotate()
			{
				struct _
				{
					static Quaternion Calc(const Vector3& position, const Quaternion& rotation, const Vector2& input, const Vector2& speed, float dt)
					{
						const Transform transform = { .position = position, .rotation = rotation };
						return TargetClass::Rotate(transform, input, speed, dt);
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
						return TargetClass::MoveH(transform, input, speed, dt);
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

			// 2x2のチャンク群を作成
			// ブロック、空気、ブロックの 3層構造
			// min, x隣, z隣, xz隣 の順に airYRanges を指定
			static HeapMultiDimAllocator::Array2D<Chunk> CreateChunk3Layerd2x2(const std::array<Lattice2, 4 >& airYRanges)
			{
				auto chunks = HeapMultiDimAllocator::CreateArray2D<Chunk>(2, 2);

				chunks[0][0] = CreateChunk3Layerd(airYRanges[0]);
				chunks[1][0] = CreateChunk3Layerd(airYRanges[1]);
				chunks[0][1] = CreateChunk3Layerd(airYRanges[2]);
				chunks[1][1] = CreateChunk3Layerd(airYRanges[3]);

				return chunks;
			}

#pragma endregion

			static constexpr Vector3 PlayerCollisionSize = Vector3(0.8f, 1.8f, 0.8f);

			static const HeapMultiDimAllocator::Array2D<Chunk>& CreateChunk3Layerd2x2ForTest()
			{
				static const auto chunks2x2 = CreateChunk3Layerd2x2({ Lattice2(4, 12), Lattice2(3, 13), Lattice2(5, 11), Lattice2(4, 12) });
				return chunks2x2;
			}

			static void Run_CreateChunk3Layerd2x2ForTest()
			{
				const auto& chunks2x2 = CreateChunk3Layerd2x2ForTest();

#define test(localChunkIndexX, localChunkIndexZ, blockPosition, expectedBlock) \
eqen(chunks2x2[localChunkIndexX][localChunkIndexZ].GetBlock(Lattice3##blockPosition), Block::##expectedBlock); \

				// x0z0
				test(0, 0, (0, 3, 0), Stone);
				test(0, 0, (0, 4, 0), Air);
				test(0, 0, (0, 12, 0), Air);
				test(0, 0, (0, 13, 0), Stone);
				test(0, 0, (12, 1, 15), Stone);
				test(0, 0, (12, 8, 15), Air);
				test(0, 0, (12, 64, 15), Stone);

				// x1z0
				test(1, 0, (0, 2, 0), Stone);
				test(1, 0, (0, 3, 0), Air);
				test(1, 0, (0, 13, 0), Air);
				test(1, 0, (0, 14, 0), Stone);
				test(1, 0, (12, 1, 15), Stone);
				test(1, 0, (12, 8, 15), Air);
				test(1, 0, (12, 64, 15), Stone);

				// x0z1
				test(0, 1, (0, 4, 0), Stone);
				test(0, 1, (0, 5, 0), Air);
				test(0, 1, (0, 11, 0), Air);
				test(0, 1, (0, 12, 0), Stone);
				test(0, 1, (12, 1, 15), Stone);
				test(0, 1, (12, 8, 15), Air);
				test(0, 1, (12, 64, 15), Stone);

				// x1z1
				test(1, 1, (0, 3, 0), Stone);
				test(1, 1, (0, 4, 0), Air);
				test(1, 1, (0, 12, 0), Air);
				test(1, 1, (0, 13, 0), Stone);
				test(1, 1, (12, 1, 15), Stone);
				test(1, 1, (12, 8, 15), Air);
				test(1, 1, (12, 64, 15), Stone);

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
	(CollisionBoundaryInfoWrapper{TargetClass::CalculateCollisionBoundaryAsBlock(Vector3##worldPositionMin, PlayerCollisionSize, Chunk::Size, 2)}), \
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
#define test(worldPositionMin, expected) \
eq( \
	(TargetClass::FindFloorHeight(CreateChunk3Layerd2x2ForTest(), Vector3##worldPositionMin, PlayerCollisionSize, Chunk::Size, 2)), \
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
#define test(worldPositionMin, expected) \
eq( \
	(TargetClass::FindCeilHeight(CreateChunk3Layerd2x2ForTest(), Vector3##worldPositionMin, PlayerCollisionSize, Chunk::Size, Chunk::Height, 2)), \
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
				test((31.4f, 8.0f, 5.0f), 14); // x1z0 で判定
				test((5.0f, 8.0f, 31.4f), 12); // x0z1 で判定
				test((31.4f, 8.0f, 31.4f), 13); // x1z1 で判定

				// チャンク配列の範囲外 (最小座標は範囲内)
				// 複数チャンクに跨っている
				// x範囲外z跨っている, z範囲外x跨っている, xz両方範囲外
				test((31.4f, 8.0f, 15.4f), 13); // x1z0, x1z1 で判定
				test((15.4f, 8.0f, 31.4f), 12); // x0z1, x1z1 で判定

				// チャンク配列の範囲外 (最小座標が範囲外)
				test((32.1f, 8.0f, 15.4f), Chunk::Height);

#undef test
			}

			static void Run_IsOverlappingWithTerrain()
			{
#define test(worldPositionMin, expected) \
eq( \
	(TargetClass::IsOverlappingWithTerrain(CreateChunk3Layerd2x2ForTest(), Vector3##worldPositionMin, PlayerCollisionSize, Chunk::Size, 2)), \
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
	}
}
