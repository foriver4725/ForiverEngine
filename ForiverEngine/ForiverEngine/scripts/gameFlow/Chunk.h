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
	/// 1チャンクの地形データ
	/// </summary>
	class Chunk
	{
	public:
		static constexpr std::uint32_t DefaultCreationSeed = 0x2961E3B1; // 生成のシード値 (デフォルト値)
		static constexpr int Size = 16;    // 1辺のサイズ (ブロック数)
		static constexpr int Height = 256; // 高さ (ブロック数)
		static constexpr int Count = 1024; // ワールド全体のチャンク数 (Count x Count 個)
		static constexpr int DrawDistance = 8; // カメラからの描画チャンク数 (矩形)
		static constexpr int DrawCountMax = DrawDistance * 2 + 1; // 描画するチャンク数の最大値 (カメラ中心に、最大 DrawCountMax x DrawCountMax 個)

		Chunk() : data(nullptr) {}
		Chunk(Chunk&& other) noexcept : data(std::move(other.data)) {}

		Chunk& operator=(Chunk&& other) noexcept
		{
			data = std::move(other.data);
			other.data = nullptr;
			return *this;
		}

		template<typename T>
		using ChunksArray = HeapMultiDimAllocator::Array2D<T>;

		template<typename T>
		using DrawChunksArray = HeapMultiDimAllocator::Array2D<T>;

		/// <summary>
		/// <para>チャンク群の数だけ要素を持った、2次元配列を作成する</para>
		/// <para>チャンクと1対1対応するデータを表現するのに使う</para>
		/// <para>アクセスは [x][z] の順</para>
		/// </summary>
		template<typename T>
		static ChunksArray<T> CreateChunksArray()
		{
			return HeapMultiDimAllocator::CreateArray2D<T>(Chunk::Count, Chunk::Count);
		}

		/// <summary>
		/// <para>描画するチャンク群の最大数だけ要素を持った、2次元配列を作成する</para>
		/// <para>実際に描画するチャンクと1対1対応するデータを表現するのに使う</para>
		/// <para>実際の描画チャンク数は最大数より少なくなり得るので、配列の全ての要素が使われるとは限らないことに注意</para>
		/// <para>アクセスは [x][z] の順</para>
		/// </summary>
		template<typename T>
		static DrawChunksArray<T> CreateDrawChunksArray()
		{
			return HeapMultiDimAllocator::CreateArray2D<T>(DrawCountMax, DrawCountMax);
		}

		struct DrawChunksIndexRangeInfo
		{
			Lattice2 rangeX; // x方向のインデックス範囲
			Lattice2 rangeZ; // z方向のインデックス範囲
			int chunkCount;  // その範囲のチャンク数合計

			// 範囲を組み替える [xMin, zMin]
			constexpr Lattice2 GetRangeMin() const noexcept
			{
				return Lattice2(rangeX.x, rangeZ.x);
			}
			// 範囲を組み替える [xMax, zMax]
			constexpr Lattice2 GetRangeMax() const noexcept
			{
				return Lattice2(rangeX.y, rangeZ.y);
			}
		};

		/// <summary>
		/// <para>実際に描画するチャンクの、インデックス範囲の情報を作成する</para>
		/// <para>チャンク配列を超える場合があるので、必ずしも最大数描画できるとは限らない</para>
		/// </summary>
		static DrawChunksIndexRangeInfo CreateDrawChunksIndexRangeInfo(const Lattice2& cameraExistingChunkIndex)
		{
			const int xMin = std::clamp(cameraExistingChunkIndex.x - DrawDistance, 0, Chunk::Count - 1);
			const int xMax = std::clamp(cameraExistingChunkIndex.x + DrawDistance, 0, Chunk::Count - 1);
			const int zMin = std::clamp(cameraExistingChunkIndex.y - DrawDistance, 0, Chunk::Count - 1);
			const int zMax = std::clamp(cameraExistingChunkIndex.y + DrawDistance, 0, Chunk::Count - 1);

			const int count = (xMax - xMin + 1) * (zMax - zMin + 1);

			return DrawChunksIndexRangeInfo
			{
				.rangeX = Lattice2(xMin, xMax),
				.rangeZ = Lattice2(zMin, zMax),
				.chunkCount = count,
			};
		}

		/// <summary>
		/// 何もない空気のみで作成する
		/// </summary>
		static Chunk CreateVoid()
		{
			Chunk chunk;

			chunk.data = HeapMultiDimAllocator::CreateArray3D<Block>(Size, Height, Size);

			for (int xi = 0; xi < Size; ++xi)
				for (int yi = 0; yi < Height; ++yi)
					for (int zi = 0; zi < Size; ++zi)
						chunk.SetBlock({ xi, yi, zi }, Block::Air);

			return chunk;
		}

		/// <summary>
		/// <para>ノイズを用いてチャンクを生成する</para>
		/// <para>高度に応じて 砂, 草/土, 石 とブロックが変化していく</para>
		/// <para>草/土 について、基本は土で、土が最上段で終わっているならそれが草になる</para>
		/// </summary>
		/// <param name="chunkIndex">チャンクのインデックス (x,z)</param>
		/// <param name="noiseScale">ノイズのスケール (x: 水平スケール, y: 垂直スケール)</param>
		/// <param name="heightBulk">この高さ分かさ増しする</param>
		/// <param name="minDirtHeight">土が出てくる最低高度</param>
		/// <param name="minStoneHeight">石が出てくる最低高度</param>
		/// <param name="seed">シード値</param>
		static Chunk CreateFromNoise(const Lattice2& chunkIndex, const Vector2& noiseScale,
			int heightBulk, int minDirtHeight, int minStoneHeight, std::uint32_t seed = DefaultCreationSeed)
		{
			Chunk chunk = CreateVoid();

			const float seedX = static_cast<float>((seed & 0xFFFF0000) >> 16);
			const float seedZ = static_cast<float>(seed & 0x0000FFFF);

			for (int x = 0; x < Size; ++x)
				for (int z = 0; z < Size; ++z)
				{
					const float noise = Noise::Simplex2D(1.0f * (x + Size * chunkIndex.x + seedX) * noiseScale.x, 1.0f * (z + Size * chunkIndex.y + seedZ) * noiseScale.x);
					const float heightNormed = (noise + 1.0f) * 0.5f; // [0, 1] に正規化
					const int height = std::clamp(heightBulk + static_cast<int>(heightNormed * noiseScale.y), 0, Height - 1);

					for (int y = 0; y <= height; ++y)
					{
						if (y >= minStoneHeight)
						{
							chunk.SetBlock({ x, y, z }, Block::Stone);
						}
						else if (y >= minDirtHeight)
						{
							if (y == height)
								chunk.SetBlock({ x, y, z }, Block::Grass); // 最上段は草
							else
								chunk.SetBlock({ x, y, z }, Block::Dirt);
						}
						else
						{
							chunk.SetBlock({ x, y, z }, Block::Sand);
						}
					}
				}

			return chunk;
		}

		Mesh CreateMesh(const Lattice2& chunkIndex) const
		{
			// データの型が違うので、一時バッファにコピーして渡す
			auto dataConverted = HeapMultiDimAllocator::CreateArray3D<std::uint32_t>(Size, Height, Size);
			for (int xi = 0; xi < Size; ++xi)
				for (int yi = 0; yi < Height; ++yi)
					for (int zi = 0; zi < Size; ++zi)
						dataConverted[xi][yi][zi] = static_cast<std::uint32_t>(data[xi][yi][zi]);

			return Mesh::CreateFromChunkData(dataConverted, { Size, Height, Size }, chunkIndex);
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
		int GetFloorHeight(const Lattice2& positionXZ, int maxY = Height - 1) const
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
			for (int y = minY; y <= Height - 1; ++y)
			{
				if (GetBlock({ positionXZ.x, y, positionXZ.y }) != Block::Air)
					return y;
			}
			return Height; // 天井が無い
		}

	private:
		// y, z, x の順番
		HeapMultiDimAllocator::Array3D<Block> data;
	};
}
