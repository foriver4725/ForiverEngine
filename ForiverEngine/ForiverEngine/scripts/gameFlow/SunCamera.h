#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// シャドウマップを作るための太陽カメラ. 平行投影
	/// </summary>
	class SunCamera
	{
	public:
		inline static const Vector3 Direction = Vector3(1.0f, -1.0f, 1.0f).Normed(); // 太陽光の向き (正規化済み)
		static constexpr Color ShadowColor = Color(0.7f, 0.7f, 0.7f);                // 影の色 (色係数)

		void LookAtPlayer(const Vector3& playerWorldFootPosition)
		{
			transform = CameraTransform::CreateOrthographic(
				playerWorldFootPosition - Direction * DistanceFromPlayer,
				Quaternion::VectorToVector(Vector3::Forward(), Direction),
				ClipSizeXY, ClipRangeZ
			);
		}

		Matrix4x4 CalculateVPMatrix() const noexcept
		{
			return transform.CalculateVPMatrix();
		}

	private:
		static constexpr float DistanceFromPlayer = 100;                             // プレイヤーからの距離
		static constexpr Vector2 ClipSizeXY = Vector2(1024, 1024);                   // クリップ領域の XY サイズ
		static constexpr Vector2 ClipRangeZ = Vector2(0.1f, 500.0f);                 // クリップ領域の Z 範囲

		CameraTransform transform;
	};
}
