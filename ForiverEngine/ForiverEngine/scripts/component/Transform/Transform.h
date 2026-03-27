#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>

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

		static constexpr Transform Identity() noexcept
		{
			return Transform
			{
				.parent = nullptr,
				.position = Vector3::Zero(),
				.rotation = Quaternion::Identity(),
				.scale = Vector3::One(),
			};
		}

		Vector3 GetRight() const noexcept
		{
			return rotation * Vector3::Right();
		}
		Vector3 GetUp() const noexcept
		{
			return rotation * Vector3::Up();
		}
		Vector3 GetForward() const noexcept
		{
			return rotation * Vector3::Forward();
		}

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

		// [シリアライズのフォーマット]
		// ※ 親子関係はシリアライズしない!!
		// 
		// float position.x
		// float position.y
		// float position.z
		// float rotation.x
		// float rotation.y
		// float rotation.z
		// float rotation.w
		// float scale.x
		// float scale.y
		// float scale.z

		static constexpr std::size_t SelfSerializedSize = sizeof(float) * 10;

		static std::string Serialize(const Transform& transform)
		{
			std::string buffer;
			buffer.resize(SelfSerializedSize);

			std::memcpy(buffer.data() + 0, &transform.position.x, sizeof(float) * 3);
			std::memcpy(buffer.data() + sizeof(float) * 3, &transform.rotation.x, sizeof(float) * 4);
			std::memcpy(buffer.data() + sizeof(float) * 7, &transform.scale.x, sizeof(float) * 3);

			return buffer;
		}

		static Transform Deserialize(std::string_view buffer)
		{
			if (buffer.size() < SelfSerializedSize)
			{
				ShowError(L"バッファのサイズが小さすぎます");
				return {};
			}

			Transform transform;

			std::memcpy(&transform.position.x, buffer.data() + 0, sizeof(float) * 3);
			std::memcpy(&transform.rotation.x, buffer.data() + sizeof(float) * 3, sizeof(float) * 4);
			std::memcpy(&transform.scale.x, buffer.data() + sizeof(float) * 7, sizeof(float) * 3);

			return transform;
		}
	};
}
