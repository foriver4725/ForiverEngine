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
		Vector2 fov; // 水平/垂直 (ラジアン)

		/// <summary>
		/// <para>View行列を計算</para>
		/// </summary>
		Matrix4x4 CalculateViewMatrix() const noexcept
		{
			Vector3 safeUp = up.Normed();
			if (std::abs(Vector3::Dot((target - position).Normed(), safeUp)) < Epsilon)
			{
				// 注視点方向とupベクトルがほぼ同じ場合、upベクトルを変更する
				safeUp = Vector3::Right();
			}

			const Vector3 zAxis = (target - position).Normed(); // 前方向
			const Vector3 xAxis = Vector3::Cross(safeUp, zAxis).Normed(); // 右方向
			const Vector3 yAxis = Vector3::Cross(zAxis, xAxis); // 上方向

			return Matrix4x4::FromColumnVectors(
				Vector4(xAxis, 0.0f),
				Vector4(yAxis, 0.0f),
				Vector4(zAxis, 0.0f),
				Vector4(
					-Vector3::Dot(xAxis, position),
					-Vector3::Dot(yAxis, position),
					-Vector3::Dot(zAxis, position),
					1.0f
				)
			);
		}

		/// <summary>
		/// <para>Projection行列を計算</para>
		/// <para>x[-1,1], y[-1,1], z[0,1]の範囲に変換</para>
		/// <para>アフィン変換ではない!!</para>
		/// </summary>
		Matrix4x4 CalculateProjectionMatrix() const noexcept
		{
			const float fovXRad = fov.x;
			const float fovYRad = fov.y;
			const float tanHalfFovX = std::tan(fovXRad * 0.5f);
			const float tanHalfFovY = std::tan(fovYRad * 0.5f);

			return Matrix4x4(
				1.0f / tanHalfFovX, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f / tanHalfFovY, 0.0f, 0.0f,
				0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f,
				0.0f, 0.0f, -(farClip * nearClip) / (farClip - nearClip), 0.0f
			);
		}
	};
}
