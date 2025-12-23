#pragma once

#include <scripts/common/Include.h>

namespace ForiverEngine
{
	enum Block : std::uint32_t
	{
		Air = 0,
		Invalid = 1,
		Grass = 2,
		Stone = 3,
		Dirt = 4,
		Sand = 5,
	};

	class Terrain
	{
	public:
		Terrain(int xSize, int ySize, int zSize)
			: data(
				std::vector<std::vector<std::vector<Block>>>(
					ySize,
					std::vector<std::vector<Block>>(
						zSize,
						std::vector<Block>(
							xSize,
							Block::Air // ‰Šú’l‚Í‹ó‹C
						)
					)
				)
			)
		{
		}

		Mesh CreateMesh() const
		{
			const std::vector<std::vector<std::vector<Block>>>* dataPtr = &data;
			const std::vector<std::vector<std::vector<std::uint32_t>>>* dataPtrAsUint
				= reinterpret_cast<const std::vector<std::vector<std::vector<std::uint32_t>>>*>(dataPtr);
			return Mesh::CreateFromTerrainData(*dataPtrAsUint);
		}

		Block GetBlock(int x, int y, int z) const
		{
			return data[y][z][x];
		}
		Block GetBlock(const Lattice3& position) const
		{
			return data[position.y][position.z][position.x];
		}

		void SetBlock(int x, int y, int z, Block block)
		{
			data[y][z][x] = block;
		}
		void SetBlock(const Lattice3& position, Block block)
		{
			data[position.y][position.z][position.x] = block;
		}

	private:
		// y, z, x ‚Ì‡”Ô
		std::vector<std::vector<std::vector<Block>>> data;
	};
}
