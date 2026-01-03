#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

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

		Terrain() : data(nullptr) {}
		Terrain(Terrain&& other) noexcept : data(std::move(other.data)) {}

		Terrain& operator=(Terrain&& other) noexcept
		{
			data = std::move(other.data);
			other.data = nullptr;
			return *this;
		}

		/// <summary>
		/// 何もない空気のみで作成する
		/// </summary>
		static Terrain CreateVoid()
		{
			Terrain terrain;

			terrain.data = HeapMultiDimAllocator::CreateArray3D<Block>(ChunkSize, ChunkHeight, ChunkSize);

			for (int xi = 0; xi < ChunkSize; ++xi)
				for (int yi = 0; yi < ChunkHeight; ++yi)
					for (int zi = 0; zi < ChunkSize; ++zi)
						terrain.SetBlock({ xi, yi, zi }, Block::Air);

			return terrain;
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
			Terrain terrain = CreateVoid();

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
							terrain.SetBlock({ x, y, z }, Block::Stone);
						}
						else if (y >= minDirtHeight)
						{
							if (y == height)
								terrain.SetBlock({ x, y, z }, Block::Grass); // 最上段は草
							else
								terrain.SetBlock({ x, y, z }, Block::Dirt);
						}
						else
						{
							terrain.SetBlock({ x, y, z }, Block::Sand);
						}
					}
				}

			return terrain;
		}

		Mesh CreateMesh(const Lattice2& localOffset) const
		{
			// データの型が違うので、一時バッファにコピーして渡す
			auto dataConverted = HeapMultiDimAllocator::CreateArray3D<std::uint32_t>(ChunkSize, ChunkHeight, ChunkSize);
			for (int xi = 0; xi < ChunkSize; ++xi)
				for (int yi = 0; yi < ChunkHeight; ++yi)
					for (int zi = 0; zi < ChunkSize; ++zi)
						dataConverted[xi][yi][zi] = static_cast<std::uint32_t>(data[xi][yi][zi]);

			return Mesh::CreateFromTerrainData(dataConverted, { ChunkSize, ChunkHeight, ChunkSize }, localOffset);
		}

		Block GetBlock(const Lattice3& position) const
		{
			return data[position.x][position.y][position.z];
		}

		void SetBlock(const Lattice3& position, Block block)
		{
			data[position.x][position.y][position.z] = block;
		}

		/// <summary>
		/// <para>地表ブロックのY座標を取得する (降順にY座標を見る. 無いならチャンクの高さの最小値-1)</para>
		/// <para>ただし、Y座標の探索については、maxY 以下しか地表候補としてみない (地中でも正しく判定するため)</para>
		/// </summary>
		int GetFloorHeight(const Lattice2& positionXZ, int maxY = ChunkHeight - 1) const
		{
			for (int y = maxY; y >= 0; --y)
			{
				if (GetBlock({ positionXZ.x, y, positionXZ.y }) != Block::Air)
					return y;
			}

			return -1; // 地面が無い
		}

		/// <summary>
		/// <para>天井ブロックのY座標を取得する (昇順にY座標を見る. 無いならチャンクの高さの最大値+1)</para>
		/// <para>ただし、Y座標の探索については、minY 以上しか天井候補としてみない (地中でも正しく判定するため)</para>
		/// </summary>
		int GetCeilHeight(const Lattice2& positionXZ, int minY = 0) const
		{
			for (int y = minY; y <= ChunkHeight - 1; ++y)
			{
				if (GetBlock({ positionXZ.x, y, positionXZ.y }) != Block::Air)
					return y;
			}
			return ChunkHeight; // 天井が無い
		}

	private:
		// y, z, x の順番
		HeapMultiDimAllocator::Array3D<Block> data;
	};
}
