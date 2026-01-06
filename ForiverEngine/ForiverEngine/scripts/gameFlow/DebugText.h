#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

namespace ForiverEngine
{
	class DebugText final
	{
	public:
		DELETE_DEFAULT_METHODS(DebugText);

		static constexpr Color Color = Color::White();

		static constexpr std::string FrameTime(double frameTime)
		{
			return std::format(
				"Frame Time : {:.2f} ms",
				frameTime
			);
		}

		static constexpr std::string Position(const PlayerController& playerController)
		{
			const Lattice3 blockPosition = playerController.GetFootBlockPosition();

			return std::format(
				"Position : {}",
				ToString(blockPosition)
			);
		}

		static constexpr std::string LookAtPosition(bool isLooking, const Lattice3& lookingBlockPosition)
		{
			if (isLooking)
				return std::format(
					"LookAt : {}",
					ToString(lookingBlockPosition)
				);
			else
				return "LookAt : None";
		}

		static constexpr std::string ChunkIndex(const PlayerController& playerController)
		{
			const Lattice3 blockPosition = playerController.GetFootBlockPosition();
			const Lattice2 chunkIndex = Chunk::GetIndex(blockPosition);

			if (Chunk::IsValidIndex(chunkIndex))
				return std::format(
					"Chunk Index : {}",
					ToString(chunkIndex)
				);
			else
				return "Chunk Index : Invalid";
		}

		static constexpr std::string ChunkLocalPosition(const PlayerController& playerController)
		{
			const Lattice3 blockPosition = playerController.GetFootBlockPosition();
			const Lattice2 chunkIndex = Chunk::GetIndex(blockPosition);
			const Lattice3 chunkLocalPosition = Chunk::GetLocalBlockPosition(blockPosition);

			if (Chunk::IsValidIndex(chunkIndex))
				return std::format(
					"Chunk Local Position : {}",
					ToString(chunkLocalPosition)
				);
			else
				return "Chunk Local Position : Invalid";
		}

		static constexpr std::string DrawChunksRange(const ChunksManager& chunksManager)
		{
			const auto& drawRangeInfo = chunksManager.GetDrawRangeInfo();

			return std::format(
				"Drawing Chunks : {}-{}",
				ToString(drawRangeInfo.GetRangeMin()),
				ToString(drawRangeInfo.GetRangeMax())
			);
		}

		static constexpr std::string CollisionRange(const PlayerController& playerController)
		{
			const Vector3 position = playerController.GetFootPosition();
			const Vector3 minPosition = PlayerControl::GetCollisionMinPosition(position, PlayerController::CollisionSize);
			const Vector3 maxPosition = minPosition + PlayerController::CollisionSize;

			return std::format(
				"Player Collision Range : {}-{}",
				ToString(PlayerControl::GetBlockPosition(minPosition)),
				ToString(PlayerControl::GetBlockPosition(maxPosition))
			);
		}

		static constexpr std::string FloorCeilHeight(const PlayerController& playerController, const ChunksManager& chunksManager)
		{
			const auto& chunks = chunksManager.GetChunks();
			const int floorHeight = playerController.FindFloorHeight(chunks);
			const int ceilHeight = playerController.FindCeilHeight(chunks);

			const std::string floorHeightText = (floorHeight >= 0) ? std::to_string(floorHeight) : "None";
			const std::string ceilHeightText = (ceilHeight <= Chunk::Height - 1) ? std::to_string(ceilHeight) : "None";

			return std::format(
				"Floor&Ceil Height : ({},{})",
				floorHeightText,
				ceilHeightText
			);
		}
	};
}
