#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./Chunk.h"
#include "./ChunksManager.h"
#include "./PlayerControl.h"

namespace ForiverEngine
{
	class PlayerController
	{
	public:
		static constexpr Vector3 CollisionSize = Vector3(0.5f, 1.8f, 0.5f); // 当たり判定サイズ
		static constexpr float GravityScale = 1.0f; // 重力の倍率
		static constexpr float SpeedH = 3.0f; // 水平移動速度 (m/s)
		static constexpr float DashSpeedH = 6.0f; // ダッシュ時の水平移動速度 (m/s)
		static constexpr float LookSensitivityH = 180.0f; // 視点の水平感度 (度/s)
		static constexpr float LookSensitivityV = 180.0f; // 視点の垂直感度 (度/s)
		static constexpr float MinVelocityV = -100.0f; // 最小鉛直速度 (m/s) - 落下速度の上限
		static constexpr float JumpHeight = 1.3f; // ジャンプの高さ (m)
		static constexpr float EyeHeight = 1.6f; // 目線の高さ (m)

		// Y方向の計算誤差を減らすためのパラメータ
		static constexpr float GroundedCheckOffset = 0.01f; // 接地判定のオフセット (m)
		static constexpr float CeilingCheckOffset = 0.01f;  // 天井判定のオフセット (m)
		static constexpr float OverlapCheckOffset = 0.001f; // 当たり判定のオフセット (m)

		static constexpr float ReachDistance = 5.0f; // リーチ距離 (m)
		static constexpr float ReachDetectStep = 0.1f; // リーチ判定時のレイステップ幅 (m)

		static constexpr float MineCooldownSeconds = 0.2f; // ブロックを掘る際のクールダウン時間 (秒)
		static constexpr float PlaceCooldownSeconds = 0.2f; // ブロックを置く際のクールダウン時間 (秒)

		PlayerController(const CameraTransform& initTransform) noexcept
			: transform(initTransform), velocityV(0.0f)
		{
		}

		Vector3 GetFootPosition() const noexcept
		{
			return PlayerControl::GetFootPosition(transform.position, EyeHeight);
		}

		Lattice3 GetFootBlockPosition() const noexcept
		{
			return PlayerControl::GetBlockPosition(GetFootPosition());
		}

		int FindFloorHeight(const Chunk::ChunksArray<Chunk>& chunks) const
		{
			return PlayerControl::FindFloorHeight(chunks, GetFootPosition() + Vector3::Up() * GroundedCheckOffset, CollisionSize);
		}

		int FindCeilHeight(const Chunk::ChunksArray<Chunk>& chunks) const
		{
			return PlayerControl::FindCeilHeight(chunks, GetFootPosition() + Vector3::Up() * -CeilingCheckOffset, CollisionSize);
		}

		bool IsOverlappingWithTerrain(const Chunk::ChunksArray<Chunk>& chunks) const
		{
			return PlayerControl::IsOverlappingWithTerrain(chunks, GetFootPosition() + Vector3::Up() * OverlapCheckOffset, CollisionSize);
		}

		bool IsOverlappingWithBlock(const Chunk::ChunksArray<Chunk>& chunks, const Lattice3& blockPosition) const
		{
			return PlayerControl::IsOverlappingWithBlock(chunks, GetFootPosition() + Vector3::Up() * OverlapCheckOffset, CollisionSize, blockPosition);
		}

		Matrix4x4 CalculateVPMatrix() const noexcept
		{
			return transform.CalculateVPMatrix();
		}

		struct Inputs
		{
			Vector2 move;
			Vector2 look;
			bool dashPressed;
			bool jumpPressed;
		};

		/// <summary>
		/// <para>毎フレーム呼び出すこと</para>
		/// <para>プレイヤーの移動・回転・ジャンプ処理を行う</para>
		/// </summary>
		void OnEveryFrame(const Chunk::ChunksArray<Chunk>& chunks, const Inputs& inputs, float deltaSeconds)
		{
			// 回転
			{
				const Quaternion rotationAmount =
					Quaternion::FromAxisAngle(Vector3::Up(), inputs.look.x * LookSensitivityH * DegToRad * deltaSeconds) *
					Quaternion::FromAxisAngle(transform.GetRight(), -inputs.look.y * LookSensitivityV * DegToRad * deltaSeconds);

				// TODO: 回転制限の計算を、もっと丁寧にやりたい!
				const Quaternion newRotation = rotationAmount * transform.rotation;
				if (std::abs((newRotation * Vector3::Forward()).y) < 0.998f) // 上下回転を制限 (前方向ベクトルのy成分で判定)
					transform.rotation = newRotation;
			}

			// 移動
			{
				// 移動前の座標を保存しておく
				const Vector3 positionBeforeMove = transform.position;

				// 鉛直移動と当たり判定
				{
					// 床のY座標を算出しておく
					const int floorY = FindFloorHeight(chunks);
					// 天井のY座標を算出しておく
					const int ceilY = FindCeilHeight(chunks);

					// 落下分の加速度を加算し、鉛直移動する
					velocityV -= (G * GravityScale) * deltaSeconds;
					velocityV = std::max(velocityV, MinVelocityV);
					if (std::abs(velocityV) > 0.01f)
						transform.position += Vector3::Up() * (velocityV * deltaSeconds);

					// 設置判定
					const bool isGrounded =
						(floorY >= 0) ?
						(GetFootPosition().y <= (floorY + 0.5f + GroundedCheckOffset))
						: false;
					// 天井判定
					const bool isCeiling =
						(ceilY <= (Chunk::Height - 1)) ?
						((GetFootPosition().y + CollisionSize.y) >= (ceilY - 0.5f - CeilingCheckOffset))
						: false;

					// 接地したら、めり込みを補正して、鉛直速度を0にする
					if (isGrounded)
					{
						const float minY = floorY + 0.5f + EyeHeight;
						if (transform.position.y < minY)
							transform.position.y = minY;

						if (velocityV < 0)
							velocityV = 0;
					}
					// 天井にぶつかったら、めり込みを補正して、鉛直速度を0にする
					else if (isCeiling)
					{
						const float maxY = ceilY - 0.5f - CollisionSize.y + EyeHeight;
						if (transform.position.y > maxY)
							transform.position.y = maxY;

						if (velocityV > 0)
							velocityV = 0;
					}

					// 接地しているなら、ジャンプ入力を受け付ける
					if (isGrounded && inputs.jumpPressed)
						velocityV += std::sqrt(2.0f * G * JumpHeight);
				}

				// 水平移動と当たり判定
				{
					const bool canDash = inputs.move.y > 0.5f; // 前進しているときのみダッシュ可能
					const float speed = (canDash && inputs.dashPressed) ? DashSpeedH : SpeedH;

					Vector3 moveDirection = transform.rotation * Vector3(inputs.move.x, 0.0f, inputs.move.y);
					moveDirection.y = 0.0f; // 水平成分のみ
					moveDirection.Norm(); // 最後に正規化する

					// X,Z それぞれで、順番に移動する
					// 移動後に地形にめり込んでいたら、移動前の位置に戻す
					// こうすることで、ブロックに対して斜めに移動した時も、動ける方向には移動できる
					{
						// X
						{
							const Vector3 positionBeforeMoveH = transform.position;

							transform.position += Vector3::Right() * (moveDirection.x * speed * deltaSeconds);

							if (IsOverlappingWithTerrain(chunks))
								transform.position = positionBeforeMoveH;
						}

						// Z
						{
							const Vector3 positionBeforeMoveH = transform.position;

							transform.position += Vector3::Forward() * (moveDirection.z * speed * deltaSeconds);

							if (IsOverlappingWithTerrain(chunks))
								transform.position = positionBeforeMoveH;
						}
					}
				}

				// 世界の範囲内に収める
				if (!PlayerControl::IsInsideWorldBounds(GetFootBlockPosition()))
				{
					transform.position = positionBeforeMove;
				}
			}
		}

		/// <summary>
		/// <para>現在見ているブロックの ブロック座標・フェースの法線 を取得する</para>
		/// <para>見ているブロックが無かったら、共に (0, 0, 0) を返す</para>
		/// </summary>
		std::pair<Lattice3, Lattice3> PickLookingBlock(const Chunk::ChunksArray<Chunk>& chunks) const
		{
			const Vector3 rayOrigin = transform.position;
			const Vector3 rayDirection = transform.GetForward();

			for (float d = 0.0f; d <= ReachDistance; d += ReachDetectStep)
			{
				const Vector3 rayPosition = rayOrigin + rayDirection * d;
				const Lattice3 rayBlockPosition = PlayerControl::GetBlockPosition(rayPosition);
				if (!PlayerControl::IsInsideWorldBounds(rayBlockPosition) ||
					!MathUtils::IsInRange(rayBlockPosition.y, 0, Chunk::Height))
					continue;

				const Lattice2 chunkIndex = Chunk::GetIndex(rayBlockPosition);
				if (!Chunk::IsValidIndex(chunkIndex))
					continue;

				const Chunk& targetingChunk = chunks[chunkIndex.x][chunkIndex.y];
				const Lattice3 rayLocalPosition = Chunk::GetLocalBlockPosition(rayBlockPosition);
				const Block blockAtRay = targetingChunk.GetBlock(rayLocalPosition);

				if (blockAtRay != Block::Air)
				{
					// フェースの法線を計算する
					// ブロック中心->レイヒット位置, フェース法線との内積を計算し、最も大きい(ベクトルが一致している)ものを採用する
					const Vector3 blockToRay = (rayPosition - Vector3(rayBlockPosition)).Normed();
					constexpr Lattice3 faceNormals[] =
					{
						Lattice3::Right(),
						Lattice3::Left(),
						Lattice3::Up(),
						Lattice3::Down(),
						Lattice3::Forward(),
						Lattice3::Backward(),
					};

					Lattice3 faceNormal = Lattice3::Zero();
					float maxDot = std::numeric_limits<float>::lowest();
					for (const Lattice3& normal : faceNormals)
					{
						// ブロックが隣接している場合、そちらのフェースは無視する
						{
							const Lattice3 adjacentBlockPosition = rayBlockPosition + normal;
							if (!PlayerControl::IsInsideWorldBounds(adjacentBlockPosition) ||
								!MathUtils::IsInRange(adjacentBlockPosition.y, 0, Chunk::Height))
								continue; // 隣接ブロックが世界の範囲外なら無視
							const Lattice2 adjacentChunkIndex = Chunk::GetIndex(adjacentBlockPosition);

							if (Chunk::IsValidIndex(adjacentChunkIndex))
							{
								const Chunk& adjacentChunk = chunks[adjacentChunkIndex.x][adjacentChunkIndex.y];
								const Lattice3 adjacentLocalPosition = Chunk::GetLocalBlockPosition(adjacentBlockPosition);
								const Block adjacentBlock = adjacentChunk.GetBlock(adjacentLocalPosition);

								if (adjacentBlock != Block::Air)
									continue; // 隣接ブロックがあるなら無視
							}
						}

						const float dot = Vector3::Dot(blockToRay, Vector3(normal));
						if (dot > maxDot)
						{
							maxDot = dot;
							faceNormal = normal;
						}
					}

					if (faceNormal == Lattice3::Zero())
					{
						// ありえないはずだが、一応チェック
						return { Lattice3::Zero(), Lattice3::Zero() };
					}

					return { rayBlockPosition, faceNormal };
				}
			}

			return { Lattice3::Zero(), Lattice3::Zero() };
		}

		/// <summary>
		/// <para>指定したブロックを掘ろうとする</para>
		/// <para>掘れたら true を返す、掘れなかったら false を返す</para>
		/// <para>掘った際にチャンクデータを更新し、描画データにも反映させる</para>
		/// <para>(この処理に device を用いる)</para>
		/// </summary>
		bool TryMineBlock(ChunksManager& chunksManager, const Lattice3& worldBlockPosition, const Device& device)
		{
			// ワールドの範囲内か?
			if (!PlayerControl::IsInsideWorldBounds(worldBlockPosition))
				return false;

			// 高度が範囲内か?
			if (!MathUtils::IsInRange(worldBlockPosition.y,
				PlayerControl::CanMinePlaceBlockHeightRange.x, PlayerControl::CanMinePlaceBlockHeightRange.y))
				return false;

			const Lattice2 chunkIndex = Chunk::GetIndex(worldBlockPosition);
			const Lattice3 localBlockPosition = Chunk::GetLocalBlockPosition(worldBlockPosition);

			// ブロックが無いならダメ (一応)
			if (chunksManager.GetChunkBlock(chunkIndex, localBlockPosition) == Block::Air)
				return false;

			const Lattice2 playerExistingChunkIndex = Chunk::GetIndex(GetFootBlockPosition());

			chunksManager.UpdateChunkBlock(chunkIndex, localBlockPosition, Block::Air, device);
			chunksManager.UpdateDrawChunks(playerExistingChunkIndex, true, device);

			return true;
		}

		/// <summary>
		/// <para>指定したブロックを置こうとする</para>
		/// <para>置けたら true を返す、置けなかったら false を返す</para>
		/// <para>置いた際にチャンクデータを更新し、描画データにも反映させる</para>
		/// <para>(この処理に device を用いる)</para>
		/// </summary>
		bool TryPlaceBlock(ChunksManager& chunksManager, const Lattice3& worldBlockPosition, const Device& device)
		{
			// ワールドの範囲内か?
			if (!PlayerControl::IsInsideWorldBounds(worldBlockPosition))
				return false;

			// 高度が範囲内か?
			if (!MathUtils::IsInRange(worldBlockPosition.y,
				PlayerControl::CanMinePlaceBlockHeightRange.x, PlayerControl::CanMinePlaceBlockHeightRange.y))
				return false;

			const Lattice2 chunkIndex = Chunk::GetIndex(worldBlockPosition);
			const Lattice3 localBlockPosition = Chunk::GetLocalBlockPosition(worldBlockPosition);

			// 既にブロックがあるならダメ (一応)
			if (chunksManager.GetChunkBlock(chunkIndex, localBlockPosition) != Block::Air)
				return false;

			// 自身の当たり判定が被っているならダメ
			if (IsOverlappingWithBlock(chunksManager.GetChunks(), worldBlockPosition))
				return false;

			const Lattice2 playerExistingChunkIndex = Chunk::GetIndex(GetFootBlockPosition());

			chunksManager.UpdateChunkBlock(chunkIndex, localBlockPosition, Block::Stone, device);
			chunksManager.UpdateDrawChunks(playerExistingChunkIndex, true, device);

			return true;
		}

	private:
		CameraTransform transform; // 一人称

		float velocityV; // 鉛直速度
	};
}
