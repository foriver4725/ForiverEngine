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

	/// <summary>
	/// チャンク単位の地形データ
	/// </summary>
	class Terrain
	{
	public:
		static constexpr int ChunkSize = 16; // チャンクの1辺のサイズ (ブロック数)
		static constexpr int ChunkHeight = 256; // チャンクの高さ (ブロック数)

		/// <summary>
		/// 何もない空気のみで作成する
		/// </summary>
		static Terrain CreateVoid(int xSize, int ySize, int zSize)
		{
			Terrain terrain;

			terrain.data = std::vector<std::vector<std::vector<Block>>>(
				ySize,
				std::vector<std::vector<Block>>(
					zSize,
					std::vector<Block>(
						xSize,
						Block::Air // 初期値は空気
					)
				)
			);

			return terrain;
		}
		/// <summary>
		/// 何もない空気のみで作成する
		/// </summary>
		static Terrain CreateVoid(const Lattice3& size)
		{
			return CreateVoid(size.x, size.y, size.z);
		}

		/// <summary>
		/// <para>ノイズを用いて地形を生成する</para>
		/// <para>高度に応じて 砂, 草/土, 石 とブロックが変化していく</para>
		/// <para>草/土 について、基本は土で、土が最上段で終わっているならそれが草になる</para>
		/// </summary>
		/// <param name="size">地形のサイズ</param>
		/// <param name="noiseScale">ノイズのスケール (x: 水平スケール, y: 垂直スケール)</param>
		/// <param name="heightBulk">この高さ分かさ増しする</param>
		/// <param name="minDirtHeight">土が出てくる最低高度</param>
		/// <param name="minStoneHeight">石が出てくる最低高度</param>
		/// <param name="seed">シード値</param>
		/// <returns></returns>
		static Terrain CreateFromNoise(const Lattice2& chunkIndex, const Vector2& noiseScale, int seed,
			int heightBulk, int minDirtHeight, int minStoneHeight)
		{
			Terrain terrain = CreateVoid(ChunkSize, ChunkHeight, ChunkSize);

			// [-32768, 32767]
			const int seedX = (seed & 0xFFFF0000) >> 16;
			const int seedZ = seed & 0x0000FFFF;

			for (int x = 0; x < ChunkSize; ++x)
				for (int z = 0; z < ChunkSize; ++z)
				{
					const float noise = Noise::Simplex2D(1.0f * (x + ChunkSize * chunkIndex.x + seedX) * noiseScale.x, 1.0f * (z + ChunkSize * chunkIndex.y + seedZ) * noiseScale.x);
					const float heightNormed = (noise + 1.0f) * 0.5f; // [0, 1] に正規化
					const int height = std::clamp(heightBulk + static_cast<int>(heightNormed * noiseScale.y), 0, ChunkHeight - 1);

					for (int y = 0; y <= height; ++y)
					{
						if (y >= minStoneHeight)
						{
							terrain.SetBlock(x, y, z, Block::Stone);
						}
						else if (y >= minDirtHeight)
						{
							if (y == height)
								terrain.SetBlock(x, y, z, Block::Grass); // 最上段は草
							else
								terrain.SetBlock(x, y, z, Block::Dirt);
						}
						else
						{
							terrain.SetBlock(x, y, z, Block::Sand);
						}
					}
				}

			return terrain;
		}

		Mesh CreateMesh(const Lattice2& localOffset) const
		{
			const std::vector<std::vector<std::vector<Block>>>* dataPtr = &data;
			const std::vector<std::vector<std::vector<std::uint32_t>>>* dataPtrAsUint
				= reinterpret_cast<const std::vector<std::vector<std::vector<std::uint32_t>>>*>(dataPtr);
			return Mesh::CreateFromTerrainData(*dataPtrAsUint, localOffset);
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

		/// <summary>
		/// 地表ブロックの高さを取得する (無いなら -1)
		/// </summary>
		int GetSurfaceHeight(int x, int z) const
		{
			for (int y = static_cast<int>(data.size()) - 1; y >= 0; --y)
			{
				if (data[y][z][x] != Block::Air)
					return y;
			}

			return -1; // 地面が無い
		}
		int GetSurfaceHeight(const Lattice2& position) const
		{
			return GetSurfaceHeight(position.x, position.y);
		}

	private:
		// y, z, x の順番
		std::vector<std::vector<std::vector<Block>>> data;
	};
}
