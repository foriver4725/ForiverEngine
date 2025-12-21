#pragma once

#include <scripts/common/Math/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// <para>カメラの Transform</para>
	/// <para>左手系</para>
	/// <para>X-右, Y-上, Z-奥</para>
	/// </summary>
	struct CameraTransform
	{
		// 親子関係にはならない

		Vector3 position;
		Vector3 lookDirection; // 必ず正規化されている前提

		float nearClip; // near > 0
		float farClip; // far > near

		bool isPerspective = true; // true: 透視投影, false: 平行投影
		float fov; // 垂直視野角 (ラジアン)
		float aspectRatio; // 幅 / 高さ

		/// <summary>
		/// <para>View行列を計算</para>
		/// </summary>
		Matrix4x4 CalculateViewMatrix() const noexcept
		{
			// 上方向のベクトルを適切に算出する
			// 基本はワールド座標系の上方向ベクトル
			Vector3 up = Vector3::Up();
			// 上を向いているなら、後方向ベクトル
			if (lookDirection == Vector3::Up())
				up = Vector3::Backward();
			// 下を向いているなら、前方向ベクトル
			else if (lookDirection == Vector3::Down())
				up = Vector3::Forward();

			// カメラ座標系の基底ベクトル
			Vector3 cameraX, cameraY, cameraZ;
			{
				cameraZ = lookDirection;
				cameraY = up; // 仮
				cameraX = Vector3::Cross(cameraY, cameraZ).Normed();
				cameraY = Vector3::Cross(cameraZ, cameraX).Normed();
			}

			const Matrix4x4 rotateInversed = Matrix4x4::FromRowVectors(
				Vector4(cameraX, 0.0f),
				Vector4(cameraY, 0.0f),
				Vector4(cameraZ, 0.0f),
				Vector4(0.0f, 0.0f, 0.0f, 1.0f)
			);
			const Matrix4x4 tranlateInversed = Matrix4x4::Translate(-position);

			return rotateInversed * tranlateInversed;
		}

		/// <summary>
		/// <para>Projection行列を計算</para>
		/// <para>x[-1,1], y[-1,1], z[0,1] の範囲に変換</para>
		/// </summary>
		Matrix4x4 CalculateProjectionMatrix() const noexcept
		{
			const float halfFov = fov * 0.5f;
			const float zRangeRcp = 1.0f / (farClip - nearClip);

			if (isPerspective)
			{
				const float xScale = 1.0f / (aspectRatio * std::tan(halfFov));
				const float yScale = 1.0f / std::tan(halfFov);
				const float zScale = farClip * zRangeRcp;
				const float zTranslate = farClip * -nearClip * zRangeRcp;

				return Matrix4x4::FromRowVectors(
					Vector4(xScale, 0.0f, 0.0f, 0.0f),
					Vector4(0.0f, yScale, 0.0f, 0.0f),
					Vector4(0.0f, 0.0f, zScale, zTranslate),
					Vector4(0.0f, 0.0f, 1.0f, 0.0f)
				);
			}
			else
			{
				// nearクリップ面で平行投影を始めると想定する
				const float xScale = 1.0f / (nearClip * aspectRatio * std::tan(halfFov));
				const float yScale = 1.0f / (nearClip * std::tan(halfFov));
				const float zScale = zRangeRcp;
				const float zTranslate = -nearClip * zRangeRcp;

				return Matrix4x4::FromRowVectors(
					Vector4(xScale, 0.0f, 0.0f, 0.0f),
					Vector4(0.0f, yScale, 0.0f, 0.0f),
					Vector4(0.0f, 0.0f, zScale, zTranslate),
					Vector4(0.0f, 0.0f, 0.0f, 1.0f)
				);
			}
		}
	};
}
