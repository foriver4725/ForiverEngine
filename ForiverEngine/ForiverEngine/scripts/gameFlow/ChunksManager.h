#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./Chunk.h"

namespace ForiverEngine
{
	class ChunksManager
	{
	public:
		static constexpr Lattice3 WorldEdgeMargin = Lattice3(2, 0, 2); // チャンクデータの端から何マスを、世界の範囲外とみなすか

		ChunksManager() = default;

		ChunksManager(const Lattice2& playerFirstExistingChunkIndex)
		{
			generationStates = Chunk::CreateChunksArray<std::atomic<ChunkGenerationState>>();
			chunks = Chunk::CreateChunksArray<Chunk>();
			meshes = Chunk::CreateChunksArray<Mesh>();
			vbvs = Chunk::CreateChunksArray<VertexBufferView>();
			ibvs = Chunk::CreateChunksArray<IndexBufferView>();

			drawVBVs = Chunk::CreateDrawChunksArray<VertexBufferView>();
			drawIBVs = Chunk::CreateDrawChunksArray<IndexBufferView>();
			drawMeshIndicesCounts = Chunk::CreateDrawChunksArray<int>();

			packedDrawVBVs.reserve(Chunk::DrawCountMax * Chunk::DrawCountMax);
			packedDrawIBVs.reserve(Chunk::DrawCountMax * Chunk::DrawCountMax);
			packedDrawMeshIndicesCounts.reserve(Chunk::DrawCountMax * Chunk::DrawCountMax);

			drawRangeInfo = Chunk::CreateDrawChunksIndexRangeInfo(playerFirstExistingChunkIndex);
		}

#pragma region Getters

		const Chunk::ChunksArray<Chunk>& GetChunks() const noexcept
		{
			return chunks;
		}
		const Chunk::DrawChunksArray<VertexBufferView>& GetDrawVBVs() const noexcept
		{
			return drawVBVs;
		}
		const Chunk::DrawChunksArray<IndexBufferView>& GetDrawIBVs() const noexcept
		{
			return drawIBVs;
		}
		const Chunk::DrawChunksArray<int>& GetDrawMeshIndicesCounts() const noexcept
		{
			return drawMeshIndicesCounts;
		}
		const Chunk::DrawChunksIndexRangeInfo& GetDrawRangeInfo() const noexcept
		{
			return drawRangeInfo;
		}

#pragma endregion

		/// <summary>
		/// <para>ワールドの範囲内であるか調べる</para>
		/// <para>チャンクデータの端にある程度近づいた時点で、範囲外判定にする</para>
		/// </summary>
		static bool IsInsideWorldBounds(const Lattice3& worldBlockPosition) noexcept
		{
			constexpr Lattice3 AllowedBlockPositionBegin = WorldEdgeMargin;
			static const Lattice3 AllowedBlockPositionEnd =
				Lattice3(Chunk::Size * Chunk::Count, Chunk::Height, Chunk::Size * Chunk::Count) - WorldEdgeMargin; // 最大値 + 1

			if (!MathUtils::IsInRange(worldBlockPosition.x, AllowedBlockPositionBegin.x, AllowedBlockPositionEnd.x))
				return false;
			// TODO: Y座標は上手くいっていない (クラッシュなどする)
			if (!MathUtils::IsInRange(worldBlockPosition.y, AllowedBlockPositionBegin.y, AllowedBlockPositionEnd.y))
				return false;
			if (!MathUtils::IsInRange(worldBlockPosition.z, AllowedBlockPositionBegin.z, AllowedBlockPositionEnd.z))
				return false;

			return true;
		}

		/// <summary>
		/// 指定されたチャンク・指定された座標のブロックを更新する
		/// その後、そのチャンクのデータを再生成する
		/// </summary>
		void UpdateChunkBlock(const Lattice2& chunkIndex, const Lattice3& localBlockPosition, const Block& newBlock, const Device& device)
		{
			chunks[chunkIndex.x][chunkIndex.y].SetBlock(localBlockPosition, newBlock);
			meshes[chunkIndex.x][chunkIndex.y] = chunks[chunkIndex.x][chunkIndex.y].CreateMesh(chunkIndex);

			const MeshViews meshViews = D3D12BasicFlow::CreateMeshViews(device, meshes[chunkIndex.x][chunkIndex.y]);
			vbvs[chunkIndex.x][chunkIndex.y] = meshViews.vbv;
			ibvs[chunkIndex.x][chunkIndex.y] = meshViews.ibv;
		};

		/// <summary>
		/// 描画するチャンクの範囲を更新し、描画するチャンクが未生成ならば新規生成する (引数で並列生成か指定可能)
		/// </summary>
		void UpdateDrawChunks(const Lattice2& playerExistingChunkIndex, bool parallelIfGenerate, const Device& deviceIfGenerate)
		{
			drawRangeInfo = Chunk::CreateDrawChunksIndexRangeInfo(playerExistingChunkIndex);

			for (int xi = drawRangeInfo.rangeX.x; xi <= drawRangeInfo.rangeX.y; ++xi)
				for (int zi = drawRangeInfo.rangeZ.x; zi <= drawRangeInfo.rangeZ.y; ++zi)
				{
					GenerateChunk({ xi, zi }, parallelIfGenerate, deviceIfGenerate);
					CopyToDrawData({ xi, zi });
				}
		}

		/// <summary>
		/// 実際に描画するものを抽出して返す
		/// </summary>
		const std::vector<VertexBufferView>& PackDrawVBVs()
		{
			PackDrawData(drawVBVs, packedDrawVBVs);
			return packedDrawVBVs;
		}
		/// <summary>
		/// 実際に描画するものを抽出して返す
		/// </summary>
		const std::vector<IndexBufferView>& PackDrawIBVs()
		{
			PackDrawData(drawIBVs, packedDrawIBVs);
			return packedDrawIBVs;
		}
		/// <summary>
		/// 実際に描画するものを抽出して返す
		/// </summary>
		const std::vector<int>& PackDrawMeshIndicesCounts()
		{
			PackDrawData(drawMeshIndicesCounts, packedDrawMeshIndicesCounts);
			return packedDrawMeshIndicesCounts;
		}

	private:
		// チャンク生成の進捗ステート
		enum class ChunkGenerationState : std::uint8_t
		{
			NotYet = 0,       // 未作成 (デフォルト値)
			CreatingParallel, // 並列処理中
			FinishedParallel, // 並列処理完了済み
			FinishedAll,      // 全部完了済み
		};

		// 全チャンクのデータ
		Chunk::ChunksArray<std::atomic<ChunkGenerationState>> generationStates;
		Chunk::ChunksArray<Chunk> chunks;
		Chunk::ChunksArray<Mesh> meshes;
		Chunk::ChunksArray<VertexBufferView> vbvs;
		Chunk::ChunksArray<IndexBufferView> ibvs;

		// 描画するチャンクのみのデータ
		Chunk::DrawChunksArray<VertexBufferView> drawVBVs;
		Chunk::DrawChunksArray<IndexBufferView> drawIBVs;
		Chunk::DrawChunksArray<int> drawMeshIndicesCounts;

		// 描画するチャンクのみのデータ (パック後. 配列を作成してキャッシュする)
		std::vector<VertexBufferView> packedDrawVBVs;
		std::vector<IndexBufferView> packedDrawIBVs;
		std::vector<int> packedDrawMeshIndicesCounts;

		// 描画するチャンクの範囲を表すデータ
		Chunk::DrawChunksIndexRangeInfo drawRangeInfo;

		// 現在いるチャンクが、描画データの配列の中でどのインデックスに対応するかを取得する
		Lattice2 GetDrawDataIndex(const Lattice2& chunkIndex) const noexcept
		{
			return chunkIndex - drawRangeInfo.GetRangeMin();
		}

		// 地形のデータ・メッシュを作成し、キャッシュする
		// 並列処理可能. 最初にこっちを実行する
		void GenerateChunkParallel(const Lattice2& chunkIndex)
		{
			Chunk chunk = Chunk::CreateFromNoise(chunkIndex, { 0.015f, 12.0f }, 16, 18, 24);

			meshes[chunkIndex.x][chunkIndex.y] = chunk.CreateMesh(chunkIndex);
			chunks[chunkIndex.x][chunkIndex.y] = std::move(chunk);

			generationStates[chunkIndex.x][chunkIndex.y].store(ChunkGenerationState::FinishedParallel, std::memory_order_release);
		};
		// ↑の並列処理を実行開始する
		void TryStartGenerateChunkParallel(const Lattice2& chunkIndex)
		{
			ChunkGenerationState expectedState = ChunkGenerationState::NotYet;

			if (!generationStates[chunkIndex.x][chunkIndex.y]
				.compare_exchange_strong(
					expectedState,
					ChunkGenerationState::CreatingParallel,
					std::memory_order_acq_rel))
			{
				// 既に該当処理が開始済みなので、何もしない
				return;
			}

			std::thread([=]()
				{
					GenerateChunkParallel(chunkIndex);
				}).detach();
		};

		// 地形の頂点・インデックスバッファビューを作成し、キャッシュしておく
		// GPUが絡むので並列処理不可. 並列処理の方が完了した後、メインスレッドで実行する
		void GenerateChunkNotParallel(const Lattice2& chunkIndex, const Device& device)
		{
			if (generationStates[chunkIndex.x][chunkIndex.y]
				.load(std::memory_order_acquire)
				!= ChunkGenerationState::FinishedParallel)
				return;

			const MeshViews meshViews = D3D12BasicFlow::CreateMeshViews(device, meshes[chunkIndex.x][chunkIndex.y]);
			vbvs[chunkIndex.x][chunkIndex.y] = meshViews.vbv;
			ibvs[chunkIndex.x][chunkIndex.y] = meshViews.ibv;

			// メインスレッドで1フレーム内で終わらせるので、この状態更新でOK
			generationStates[chunkIndex.x][chunkIndex.y].store(ChunkGenerationState::FinishedAll, std::memory_order_release);
		};

		// 指定されたチャンクを生成する
		// 並列で処理するか、指定できる
		void GenerateChunk(const Lattice2& chunkIndex, bool parallel, const Device& device)
		{
			if (parallel)
			{
				// 作成中にやっぱり描画しないとなっても、スレッドは止まらず並列処理完了まで動き続ける
				// そのため、並列処理でない部分をその後いつ呼んでも問題ない (状態ガードをちゃんと入れているので)
				TryStartGenerateChunkParallel(chunkIndex);
				GenerateChunkNotParallel(chunkIndex, device);
			}
			else
			{
				// メインスレッドで1フレームで全て終わらせる
				GenerateChunkParallel(chunkIndex);
				GenerateChunkNotParallel(chunkIndex, device);
			}
		}

		// 指定されたチャンクについて、描画するデータに値をコピーする
		// 強制上書きするので、描画データからアンセットする処理はない. ただし、描画するチャンクインデックスを全て列挙し、その中でこのメソッドを呼ぶこと
		void CopyToDrawData(const Lattice2& chunkIndex)
		{
			const Lattice2 drawDataIndex = GetDrawDataIndex(chunkIndex);

			drawVBVs[drawDataIndex.x][drawDataIndex.y] = vbvs[chunkIndex.x][chunkIndex.y];
			drawIBVs[drawDataIndex.x][drawDataIndex.y] = ibvs[chunkIndex.x][chunkIndex.y];
			drawMeshIndicesCounts[drawDataIndex.x][drawDataIndex.y] =
				static_cast<int>(meshes[chunkIndex.x][chunkIndex.y].indices.size());
		};

		// 描画データの中から実際に描画するもののみを抽出し、1次元配列にパックする
		template<typename T>
		void PackDrawData(const Chunk::DrawChunksArray<T>& drawData, std::vector<T>& outPackedDrawData)
		{
			outPackedDrawData.clear();
			for (int xi = drawRangeInfo.rangeX.x; xi <= drawRangeInfo.rangeX.y; ++xi)
				for (int zi = drawRangeInfo.rangeZ.x; zi <= drawRangeInfo.rangeZ.y; ++zi)
				{
					const Lattice2 drawDataIndex = GetDrawDataIndex({ xi, zi });
					outPackedDrawData.push_back(drawData[drawDataIndex.x][drawDataIndex.y]);
				}
		}
	};
}
