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
		static constexpr float LookSensitivityV = 90.0f; // 視点の垂直感度 (度/s)
		static constexpr float MinVelocityV = -100.0f; // 最小鉛直速度 (m/s) - 落下速度の上限
		static constexpr float JumpHeight = 1.3f; // ジャンプの高さ (m)
		static constexpr float EyeHeight = 1.6f; // 目線の高さ (m)
		static constexpr float GroundedCheckOffset = 0.01f; // 接地判定のオフセット (m). 埋まっている判定と区別するため、少しずらす
		static constexpr float CeilingCheckOffset = 0.01f;  // 天井判定のオフセット (m). 埋まっている判定と区別するため、少しずらす

		static constexpr float ReachDistance = 5.0f; // リーチ距離 (m)
		static constexpr float ReachDetectStep = 0.1f; // リーチ判定時のレイステップ幅 (m)

		PlayerController(const CameraTransform& initTransform) noexcept
			: transform(initTransform), velocityV(0.0f)
		{
		}

		const CameraTransform& GetTransform() const noexcept
		{
			return transform;
		}

		Vector3 GetFootPosition() const noexcept
		{
			return PlayerControl::GetFootPosition(transform.position, EyeHeight);
		}

		Lattice3 GetFootBlockPosition() const noexcept
		{
			return PlayerControl::GetBlockPosition(GetFootPosition());
		}

		std::pair<Vector3, Vector3> GetCollisionRange() const noexcept
		{
			const Vector3 minPosition = PlayerControl::GetCollisionMinPosition(GetFootPosition(), CollisionSize);
			const Vector3 maxPosition = minPosition + CollisionSize;

			return { minPosition, maxPosition };
		}

		int FindFloorHeight(const Chunk::ChunksArray<Chunk>& chunks) const
		{
			return PlayerControl::FindFloorHeight(chunks, GetFootPosition(), CollisionSize);
		}

		int FindCeilHeight(const Chunk::ChunksArray<Chunk>& chunks) const
		{
			return PlayerControl::FindCeilHeight(chunks, GetFootPosition(), CollisionSize);
		}

		bool IsOverlappingWithTerrain(const Chunk::ChunksArray<Chunk>& chunks) const
		{
			return PlayerControl::IsOverlappingWithTerrain(chunks, GetFootPosition(), CollisionSize);
		}

		struct Inputs
		{
			Vector2 move;
			Vector2 look;
			bool dashPressed;
			bool jumpPressedNow;
		};

		void OnEveryFrame(const Chunk::ChunksArray<Chunk>& chunks, const Inputs& inputs, float deltaSeconds)
		{
			// 回転
			{
				const Quaternion rotationAmount =
					Quaternion::FromAxisAngle(Vector3::Up(), inputs.look.x * LookSensitivityH * DegToRad * deltaSeconds) *
					Quaternion::FromAxisAngle(transform.GetRight(), -inputs.look.y * LookSensitivityV * DegToRad * deltaSeconds);

				const Quaternion newRotation = rotationAmount * transform.rotation;
				if (std::abs((newRotation * Vector3::Forward()).y) < 0.999f) // 上下回転を制限 (前方向ベクトルのy成分で判定)
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
						const float footY = floorY + 0.5f + EyeHeight;
						if (transform.position.y < footY)
							transform.position.y = footY;

						if (velocityV < 0)
							velocityV = 0;
					}
					// 天井にぶつかったら、めり込みを補正して、鉛直速度を0にする
					else if (isCeiling)
					{
						const float headY = ceilY - 0.5f - CollisionSize.y + EyeHeight;
						if (transform.position.y > headY)
							transform.position.y = headY;

						if (velocityV > 0)
							velocityV = 0;
					}

					// 接地しているなら、ジャンプ入力を受け付ける
					if (isGrounded)
					{
						if (inputs.jumpPressedNow)
							velocityV += std::sqrt(2.0f * G * JumpHeight);
					}
				}

				// 水平移動と当たり判定
				{
					// 移動前の座標を保存しておく
					const Vector3 positionBeforeMoveH = transform.position;

					const bool canDash = inputs.move.y > 0.5f; // 前進しているときのみダッシュ可能
					const float speed = (canDash && inputs.dashPressed) ? DashSpeedH : SpeedH;

					Vector3 moveDirection = transform.rotation * Vector3(inputs.move.x, 0.0f, inputs.move.y);
					moveDirection.y = 0.0f; // 水平成分のみ
					moveDirection.Norm(); // 最後に正規化する

					transform.position += moveDirection * (speed * deltaSeconds);

					// 当たり判定
					// めり込んでいるなら、元の位置に戻す
					if (IsOverlappingWithTerrain(chunks))
					{
						transform.position = positionBeforeMoveH;
					}
				}

				// 世界の範囲内に収める
				if (!ChunksManager::IsInsideWorldBounds(GetFootBlockPosition()))
				{
					transform.position = positionBeforeMove;
				}
			}
		}

	private:
		CameraTransform transform; // 一人称

		float velocityV; // 鉛直速度
	};
}
