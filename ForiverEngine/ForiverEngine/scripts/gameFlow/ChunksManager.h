#pragma once

#include "scripts/gameFlow/IncludeInternal.h"
#include "./Chunk.h"

namespace ForiverEngine
{
	class ChunksManager
	{
	public:
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

			renderMeshContext = {};
			renderMeshContext.vbvList.reserve(Chunk::DrawCountMax * Chunk::DrawCountMax);
			renderMeshContext.ibvList.reserve(Chunk::DrawCountMax * Chunk::DrawCountMax);
			renderMeshContext.indexCountList.reserve(Chunk::DrawCountMax * Chunk::DrawCountMax);

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
		/// 指定されたワールド座標のブロックを取得する
		/// </summary>
		Block GetBlock(const Lattice3& worldBlockPosition) const
		{
			const Lattice2 chunkIndex = Chunk::GetIndex(worldBlockPosition);
			const Lattice3 localBlockPosition = Chunk::GetLocalBlockPosition(worldBlockPosition);
			return GetBlock(chunkIndex, localBlockPosition);
		};

		/// <summary>
		/// 指定されたチャンク・ローカル座標のブロックを取得する
		/// </summary>
		Block GetBlock(const Lattice2& chunkIndex, const Lattice3& localBlockPosition) const
		{
			return chunks[chunkIndex.x][chunkIndex.y].GetBlock(localBlockPosition);
		};

		/// <summary>
		/// <para>指定されたワールド座標のブロックを更新する</para>
		/// <para>その後、そのチャンクのデータを再生成する</para>
		/// </summary>
		void UpdateBlock(const Lattice3& worldBlockPosition, Block newBlock, const Device& device)
		{
			const Lattice2 chunkIndex = Chunk::GetIndex(worldBlockPosition);
			const Lattice3 localBlockPosition = Chunk::GetLocalBlockPosition(worldBlockPosition);
			UpdateBlock(chunkIndex, localBlockPosition, newBlock, device);
		};

		/// <summary>
		/// <para>指定されたチャンク・ローカル座標のブロックを更新する</para>
		/// <para>その後、そのチャンクのデータを再生成する</para>
		/// </summary>
		void UpdateBlock(const Lattice2& chunkIndex, const Lattice3& localBlockPosition, Block newBlock, const Device& device)
		{
			chunks[chunkIndex.x][chunkIndex.y].SetBlock(localBlockPosition, newBlock);
			meshes[chunkIndex.x][chunkIndex.y] = chunks[chunkIndex.x][chunkIndex.y].CreateMesh(chunkIndex);

			const auto [vbv, ibv] = meshes[chunkIndex.x][chunkIndex.y].CreateRenderViews(device);
			vbvs[chunkIndex.x][chunkIndex.y] = vbv;
			ibvs[chunkIndex.x][chunkIndex.y] = ibv;
		};

		/// <summary>
		/// <para>描画するチャンクの範囲を更新し、描画するチャンクが未生成ならば新規生成する (引数で並列生成か指定可能)</para>
		/// <para>また、描画データに値をコピーする</para>
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
		/// <para>実際に描画するものだけ抽出し、RenderMeshContext を更新する</para>
		/// <para>そして、その RenderMeshContext への参照を返す</para>
		/// </summary>
		const RenderMeshContext& PackToRenderMeshContext()
		{
			PackDrawData(drawVBVs, renderMeshContext.vbvList);
			PackDrawData(drawIBVs, renderMeshContext.ibvList);
			PackDrawData(drawMeshIndicesCounts, renderMeshContext.indexCountList);

			return renderMeshContext;
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

		// 描画するチャンクのみのデータ
		// パックされたもので、実際の描画ではこれを用いる
		// 配列を作成してキャッシュする
		RenderMeshContext renderMeshContext;

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

			const auto [vbv, ibv] = meshes[chunkIndex.x][chunkIndex.y].CreateRenderViews(device);
			vbvs[chunkIndex.x][chunkIndex.y] = vbv;
			ibvs[chunkIndex.x][chunkIndex.y] = ibv;

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
