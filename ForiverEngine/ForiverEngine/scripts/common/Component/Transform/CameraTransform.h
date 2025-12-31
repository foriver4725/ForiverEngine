#pragma once

#include <scripts/common/Math/Include.h>
#include "./Transform.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>カメラの Transform</para>
	/// <para>左手系</para>
	/// <para>X-右, Y-上, Z-奥</para>
	/// </summary>
	struct CameraTransform : public Transform
	{
		// 親子関係にはならない

		float nearClip; // near > 0
		float farClip; // far > near

		bool isPerspective = true; // true: 透視投影, false: 平行投影
		float fov; // 垂直視野角 (ラジアン)
		float aspectRatio; // 幅 / 高さ

		static CameraTransform CreatePerspective(const Vector3& position, const Quaternion& rotation, float fov, float aspectRatio,
			float nearClip = 0.1f, float farClip = 1000.0f
		) noexcept
		{
			CameraTransform cameraTransform = {};

			// Transform
			cameraTransform.parent = nullptr;
			cameraTransform.position = position;
			cameraTransform.rotation = rotation;
			cameraTransform.scale = Vector3::One();

			// CameraTransform
			cameraTransform.nearClip = nearClip;
			cameraTransform.farClip = farClip;
			cameraTransform.isPerspective = true;
			cameraTransform.fov = fov;
			cameraTransform.aspectRatio = aspectRatio;

			return cameraTransform;
		}

		static CameraTransform CreateOrthographic(const Vector3& position, const Quaternion& rotation, float clipSizeX, float clipSizeY,
			float nearClip = 0.1f, float farClip = 1000.0f
		) noexcept
		{
			CameraTransform cameraTransform = {};

			// Transform
			cameraTransform.parent = nullptr;
			cameraTransform.position = position;
			cameraTransform.rotation = rotation;
			cameraTransform.scale = Vector3::One();

			// CameraTransform
			cameraTransform.nearClip = nearClip;
			cameraTransform.farClip = farClip;
			cameraTransform.isPerspective = false;
			cameraTransform.fov = std::atan2(clipSizeY * 0.5f, nearClip) * 2.0f; // nearクリップ面で平行投影を始めると想定するので...
			cameraTransform.aspectRatio = clipSizeX / clipSizeY;

			return cameraTransform;
		}

		/// <summary>
		/// <para>View行列を計算</para>
		/// </summary>
		Matrix4x4 CalculateViewMatrix() const noexcept
		{
			const Matrix4x4 rotateInversed = Matrix4x4::FromRowVectors(
				Vector4(GetRight(), 0.0f),
				Vector4(GetUp(), 0.0f),
				Vector4(GetForward(), 0.0f),
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

		/// <summary>
		/// VP行列を計算
		/// </summary>
		Matrix4x4 CalculateVPMatrix() const noexcept
		{
			const Matrix4x4 v = CalculateViewMatrix();
			const Matrix4x4 p = CalculateProjectionMatrix();

			return p * v;
		}
	};
}
