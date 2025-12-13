#pragma once

#include "./Defines.h"

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
		Vector3 target;
		Vector3 up = Vector3::Up();

		float nearClip; // near > 0
		float farClip; // far > near

		bool isPerspective = true; // true: 透視投影, false: 平行投影
		float fov; // 垂直方向 (ラジアン. 視線ベクトルからの角度)
		float aspectRatio; // 幅 / 高さ

		/// <summary>
		/// <para>View行列を計算</para>
		/// </summary>
		Matrix4x4 CalculateViewMatrix() const noexcept
		{
			const Vector3 toTarget = target - position;
			Vector3 up = this->up;

			// 注視点に近すぎる → 単位行列を返す
			if (toTarget.LenSq() < Epsilon)
				return Matrix4x4::Identity();
			// up ベクトルが視線ベクトルに近すぎる → 別の軸をupにする
			if (Vector3::Dot(toTarget.Normed(), up.Normed()) > 1.0f - Epsilon)
				up = (std::abs(toTarget.y) < 0.9f) ? Vector3::Up() : Vector3::Right();

			// カメラ座標系の基底ベクトル
			Vector3 cameraX, cameraY, cameraZ;
			{
				cameraZ = toTarget.Normed();
				cameraY = up.Normed(); // 仮
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
			const float zRangeRcp = 1.0f / (farClip - nearClip);

			if (isPerspective)
			{
				const float xScale = 1.0f / (aspectRatio * std::tan(fov));
				const float yScale = 1.0f / std::tan(fov);
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
				const float xScale = 1.0f / (nearClip * aspectRatio * std::tan(fov));
				const float yScale = 1.0f / (nearClip * std::tan(fov));
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
