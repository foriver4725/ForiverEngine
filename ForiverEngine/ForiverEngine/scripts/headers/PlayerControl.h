#pragma once

#include <scripts/common/Include.h>
#include "./Terrain.h"

namespace ForiverEngine
{
	class PlayerControl final
	{
	public:
		DELETE_DEFAULT_METHODS(PlayerControl);

		static Vector3 GetFootPosition(const Vector3& position, float eyeHeight)
		{
			return position - Vector3::Up() * eyeHeight;
		}

		static void Rotate(Transform& transform, const Vector2& rotateInput, const Vector2& rotateSpeed, float deltaSeconds)
		{
			const Quaternion rotationAmount =
				Quaternion::FromAxisAngle(Vector3::Up(), rotateInput.x * rotateSpeed.x * deltaSeconds) *
				Quaternion::FromAxisAngle(transform.GetRight(), -rotateInput.y * rotateSpeed.y * deltaSeconds);

			const Quaternion newRotation = rotationAmount * transform.rotation;

			if (std::abs((newRotation * Vector3::Forward()).y) < 0.99f) // 上下回転を制限 (前方向ベクトルのy成分で判定)
				transform.rotation = newRotation;
		}

		static void MoveH(Transform& transform, const Vector2& moveInput, float moveSpeed, float deltaSeconds)
		{
			Vector3 moveDirection = transform.rotation * Vector3(moveInput.x, 0.0f, moveInput.y);
			moveDirection.y = 0.0f; // 水平成分のみ
			moveDirection.Norm(); // 最後に正規化する

			transform.position += moveDirection * (moveSpeed * deltaSeconds);
		}

		// 足元にある中で、最も高い地面の高さを取得する
		static int GetFootSurfaceHeight(
			const Terrain& terrain,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const int minX = static_cast<int>(std::round(position.x - size.x * 0.5f));
			const int maxX = static_cast<int>(std::round(position.x + size.x * 0.5f));
			const int minZ = static_cast<int>(std::round(position.z - size.z * 0.5f));
			const int maxZ = static_cast<int>(std::round(position.z + size.z * 0.5f));

			int surfaceY = -1;
			for (int z = minZ; z <= maxZ; ++z)
				for (int x = minX; x <= maxX; ++x)
					surfaceY = std::max(surfaceY, terrain.GetSurfaceHeight(x, z));

			return surfaceY;
		}

		static bool IsOverlappingWithTerrain(
			const Terrain& terrain,
			const Vector3& position, // 足元の座標
			const Vector3& size // コリジョンのサイズ
		)
		{
			const int minX = static_cast<int>(std::round(position.x - size.x * 0.5f));
			const int maxX = static_cast<int>(std::round(position.x + size.x * 0.5f));
			const int minY = static_cast<int>(std::round(position.y));
			const int maxY = static_cast<int>(std::round(position.y + size.y));
			const int minZ = static_cast<int>(std::round(position.z - size.z * 0.5f));
			const int maxZ = static_cast<int>(std::round(position.z + size.z * 0.5f));

			for (int y = minY; y <= maxY; ++y)
				for (int z = minZ; z <= maxZ; ++z)
					for (int x = minX; x <= maxX; ++x)
					{
						if (terrain.GetBlock(x, y, z) != Block::Air)
							return true;
					}

			return false;
		}
	};
}
