#pragma once

#include "./Defines.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>基本的なオブジェクトの Transform</para>
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

		/// <summary>
		/// <para>Model行列を計算</para>
		/// </summary>
		Matrix4x4 CalculateModelMatrix() const noexcept
		{
			const Matrix4x4 s = Matrix4x4::Scale(scale);
			const Matrix4x4 r = Matrix4x4::Rotate(rotation);
			const Matrix4x4 t = Matrix4x4::Translate(position);

			const Matrix4x4 localMatrix = t * r * s;
			const Matrix4x4 worldMatrix = parent ? parent->CalculateModelMatrix() * localMatrix : localMatrix;

			return worldMatrix;
		}

		/// <summary>
		/// <para>Modelの逆行列を計算</para>
		/// </summary>
		Matrix4x4 CalculateModelMatrixInversed() const noexcept
		{
			const Matrix4x4 sInv = Matrix4x4::Scale(Vector3(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z));
			const Matrix4x4 rInv = Matrix4x4::Rotate(rotation.Conjugate());
			const Matrix4x4 tInv = Matrix4x4::Translate(-position);

			const Matrix4x4 localMatrixInv = sInv * rInv * tInv;
			const Matrix4x4 worldMatrixInv = parent ? localMatrixInv * parent->CalculateModelMatrixInversed() : localMatrixInv;

			return worldMatrixInv;
		}
	};
}
