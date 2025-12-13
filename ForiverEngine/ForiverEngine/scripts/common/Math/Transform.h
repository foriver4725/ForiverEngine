#pragma once

#include "./Defines.h"
#include "./LinearAlgebra/Include.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>左手系</para>
	/// <para>X-右, Y-上, Z-奥</para>
	/// </summary>
	struct Transform
	{
		// 親子関係
		const Transform* parent = nullptr;

		// ローカル値
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;

		// モデル行列を計算
		Matrix4x4 CalculateModelMatrix() const noexcept
		{
			const Matrix4x4 s = Matrix4x4::Scale(scale);
			const Matrix4x4 r = Matrix4x4::Rotate(rotation);
			const Matrix4x4 t = Matrix4x4::Translate(position);

			const Matrix4x4 localMatrix = t * r * s;
			const Matrix4x4 worldMatrix = parent ? parent->CalculateModelMatrix() * localMatrix : localMatrix;

			return worldMatrix;
		}
	};
}
