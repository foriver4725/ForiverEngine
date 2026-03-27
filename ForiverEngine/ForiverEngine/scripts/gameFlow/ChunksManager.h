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

			// 適当に reserve
			dirtyChunkBinaries.reserve(1024);
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
		}

		/// <summary>
		/// 指定されたチャンク・ローカル座標のブロックを取得する
		/// </summary>
		Block GetBlock(const Lattice2& chunkIndex, const Lattice3& localBlockPosition) const
		{
			return chunks[chunkIndex.x][chunkIndex.y].GetBlock(localBlockPosition);
		}

		/// <summary>
		/// <para>指定されたワールド座標のブロックを更新する</para>
		/// <para>その後、そのチャンクのデータを再生成する</para>
		/// </summary>
		void UpdateBlock(const Lattice3& worldBlockPosition, Block newBlock, const Device& device)
		{
			const Lattice2 chunkIndex = Chunk::GetIndex(worldBlockPosition);
			const Lattice3 localBlockPosition = Chunk::GetLocalBlockPosition(worldBlockPosition);
			UpdateBlock(chunkIndex, localBlockPosition, newBlock, device);
		}

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

			// 変更したチャンクのデータをシリアライズして保存する
			std::string binaryData = chunks[chunkIndex.x][chunkIndex.y].Serialize();
			dirtyChunkBinaries[chunkIndex] = std::move(binaryData);
		}

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

		// 任意の手段で情報が変化したチャンクを、シリアライズして保存する (セーブデータの実現)
		// チャンクインデックス -> シリアライズされたバイナリデータ のハッシュマップ
		std::unordered_map<Lattice2, std::string> dirtyChunkBinaries;

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
		}
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
		}

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
		}

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
		}

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

		// [セーブデータの形式]
		// 
		// [先頭]
		// std::uint64_t : チャンクの数
		// 
		// [チャンクの数だけ繰り返し]
		// std::uint64_t : チャンクインデックス (x)
		// std::uint64_t : チャンクインデックス (y)
		// std::uint64_t : シリアライズされたバイナリデータ(文字列)のサイズ
		// char[] : シリアライズされたバイナリデータ(文字列). サイズは前の項目で指定されたもの

		static bool SaveChunks(std::ostream& os, const std::unordered_map<Lattice2, std::string>& serializedChunks)
		{
			try
			{
				const std::uint64_t chunkCount = static_cast<std::uint64_t>(serializedChunks.size());
				if (!os.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount))) return false;

				for (const auto& [chunkIndex, binary] : serializedChunks)
				{
					const std::uint64_t x = static_cast<std::uint64_t>(chunkIndex.x);
					const std::uint64_t y = static_cast<std::uint64_t>(chunkIndex.y);
					const std::uint64_t binarySize = static_cast<std::uint64_t>(binary.size());

					if (!os.write(reinterpret_cast<const char*>(&x), sizeof(x))) return false;
					if (!os.write(reinterpret_cast<const char*>(&y), sizeof(y))) return false;
					if (!os.write(reinterpret_cast<const char*>(&binarySize), sizeof(binarySize))) return false;

					if (binarySize > 0)
					{
						if (!os.write(binary.data(), static_cast<std::streamsize>(binarySize))) return false;
					}
				}

				return true;
			}
			catch (...)
			{
				return false;
			}
		}

		static bool LoadDirtyChunks(std::istream& is, std::unordered_map<Lattice2, std::string>& serializedChunks)
		{
			try
			{
				std::uint64_t chunkCount = 0;
				if (!is.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount))) return false;

				std::unordered_map<Lattice2, std::string> output;
				output.reserve(static_cast<std::size_t>(chunkCount));

				for (std::uint64_t i = 0; i < chunkCount; ++i)
				{
					std::uint64_t x = 0;
					std::uint64_t y = 0;
					std::uint64_t binarySize = 0;

					if (!is.read(reinterpret_cast<char*>(&x), sizeof(x))) return false;
					if (!is.read(reinterpret_cast<char*>(&y), sizeof(y))) return false;
					if (!is.read(reinterpret_cast<char*>(&binarySize), sizeof(binarySize))) return false;

					std::string binary;
					if (binarySize > 0)
					{
						binary.resize(static_cast<std::size_t>(binarySize));
						if (!is.read(binary.data(), static_cast<std::streamsize>(binarySize))) return false;
					}

					const Lattice2 chunkIndex = Lattice2(static_cast<int>(x), static_cast<int>(y));
					output[chunkIndex] = std::move(binary);
				}

				serializedChunks = std::move(output);

				return true;
			}
			catch (...)
			{
				return false;
			}
		}
	};
}
