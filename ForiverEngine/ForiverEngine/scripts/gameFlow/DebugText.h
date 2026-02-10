#pragma once

#include "scripts/gameFlow/IncludeInternal.h"
#include "./PlayerController.h"
#include "./ChunksManager.h"
#include "./Chunk.h"

namespace ForiverEngine
{
	class DebugText final
	{
	public:
		DELETE_DEFAULT_METHODS(DebugText);

		// [ms]
		struct FrameTimeBreakdown
		{
			double preFrame;
			double cpu;
			double gpu;
			double postFrame;
			double total;
		};

		// [ms]
		static std::string FrameTime(const FrameTimeBreakdown& frameTimeBreakdown)
		{
			return std::format(
				"Frame Time[ms] : Total={:.2f}(PreFrame={:.2f},CPU={:.2f},GPU={:.2f},PostFrame={:.2f})",
				frameTimeBreakdown.total,
				frameTimeBreakdown.preFrame, frameTimeBreakdown.cpu, frameTimeBreakdown.gpu, frameTimeBreakdown.postFrame
			);
		}

		static std::string Position(const PlayerController& playerController)
		{
			const Lattice3 blockPosition = playerController.GetFootBlockPosition();

			return std::format(
				"Position : {}",
				ToString(blockPosition)
			);
		}

		struct LookingBlockInfo
		{
			bool isLooking;
			Lattice3 lookingBlockWorldPosition;
			Lattice3 lookingBlockFaceNormal;
		};
		static std::string LookingBlock(const LookingBlockInfo& info, const ChunksManager& chunksManager)
		{
			if (info.isLooking)
				return std::format(
					"Looking Block : {}(At={},Face={})",
					GetBlockName(chunksManager.GetBlock(info.lookingBlockWorldPosition)),
					ToString(info.lookingBlockWorldPosition),
					ToString(info.lookingBlockFaceNormal)
				);
			else
				return "Looking Block : None";
		}

		static std::string ChunkIndex(const PlayerController& playerController)
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

		static std::string ChunkLocalPosition(const PlayerController& playerController)
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

		static std::string DrawChunksRange(const ChunksManager& chunksManager)
		{
			const auto& drawRangeInfo = chunksManager.GetDrawRangeInfo();

			return std::format(
				"Drawing Chunks : {}-{}",
				ToString(drawRangeInfo.GetRangeMin()),
				ToString(drawRangeInfo.GetRangeMax())
			);
		}

		static std::string CollisionRange(const PlayerController& playerController)
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

		static std::string FloorCeilHeight(const PlayerController& playerController, const ChunksManager& chunksManager)
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
