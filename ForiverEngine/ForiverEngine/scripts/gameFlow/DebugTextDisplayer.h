#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./Renderer/UI/TextRenderer.h"
#include "./PlayerController.h"
#include "./ChunksManager.h"
#include "./DebugFrameTimeStats.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>デバッグ用テキストを画面に表示するためのクラス</para>
	/// <para>1行ごとの(文字・色)のデータを保持し、</para>
	/// <para>文字列の生成・Renderer への描画指示を行う</para>
	/// </summary>
	class DebugTextDisplayer final
	{
	public:
		static constexpr Color TextColor = Color::White();

		explicit DebugTextDisplayer()
		{
			// 適当に reserve しておく
			rowDatas.reserve(64);
		}

		struct DebugFrameTimeStatsBreakdown
		{
			const DebugFrameTimeStats& preFrame;
			const DebugFrameTimeStats& cpu;
			const DebugFrameTimeStats& gpu;
			const DebugFrameTimeStats& postFrame;
		};

		void UpdateDataAsFold(
			// Renderer. データを更新し、GPU にも反映させる
			TextRenderer& textRenderer,
			// Renderer に対する処理で必要な、描画オブジェクト
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator
		)
		{
			rowDatas.clear();
			rowDatas.emplace_back("Place F1 to unfold debug info.", TextColor);

			ApplyDataToRenderer(rowDatas, textRenderer);
			textRenderer.UpdateDataAtGPU(device, commandList, commandQueue, commandAllocator);
		}

		void UpdateDataAsUnfold(
			// Renderer. データを更新し、GPU にも反映させる
			TextRenderer& textRenderer,
			// Renderer に対する処理で必要な、描画オブジェクト
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			// 多くの処理で共通して使う
			const PlayerController& playerController, const ChunksManager& chunksManager,
			// 以下は個別の処理で使う
			const DebugFrameTimeStatsBreakdown& frameTimeStatsBreakdown,
			const DebugText::LookAtInfo& lookAtInfo
		)
		{
			const double frameTimePreFrame = frameTimeStatsBreakdown.preFrame.CalculateMean();
			const double frameTimeCPU = frameTimeStatsBreakdown.cpu.CalculateMean();
			const double frameTimeGPU = frameTimeStatsBreakdown.gpu.CalculateMean();
			const double frameTimePostFrame = frameTimeStatsBreakdown.postFrame.CalculateMean();
			const double frameTimeTotal = frameTimePreFrame + frameTimeCPU + frameTimeGPU + frameTimePostFrame;
			const DebugText::FrameTimeBreakdown frameTimeBreakdown =
			{
				.preFrame = frameTimePreFrame,
				.cpu = frameTimeCPU,
				.gpu = frameTimeGPU,
				.postFrame = frameTimePostFrame,
				.total = frameTimeTotal,
			};

			rowDatas.clear();
			rowDatas.emplace_back(DebugText::FrameTime(frameTimeBreakdown), TextColor);                    // 0
			rowDatas.emplace_back(DebugText::Position(playerController), TextColor);                       // 1
			rowDatas.emplace_back(DebugText::LookAtPosition(lookAtInfo), TextColor);                       // 2
			rowDatas.emplace_back(DebugText::ChunkIndex(playerController), TextColor);                     // 3
			rowDatas.emplace_back(DebugText::ChunkLocalPosition(playerController), TextColor);             // 4
			rowDatas.emplace_back(DebugText::DrawChunksRange(chunksManager), TextColor);                   // 5
			rowDatas.emplace_back(DebugText::CollisionRange(playerController), TextColor);                 // 6
			rowDatas.emplace_back(DebugText::FloorCeilHeight(playerController, chunksManager), TextColor); // 7

			ApplyDataToRenderer(rowDatas, textRenderer);
			textRenderer.UpdateDataAtGPU(device, commandList, commandQueue, commandAllocator);
		}

	private:
		struct RowData
		{
			std::string text;
			Color color;
		};

		// 1行ごとに文字のデータを保持する
		std::vector<RowData> rowDatas;

		/// <summary>
		/// Renderer にデータを反映させる
		/// </summary>
		static void ApplyDataToRenderer(const std::vector<RowData>& rowDatas, TextRenderer& textRenderer)
		{
			textRenderer.data.ClearAll();
			for (int i = 0; i < static_cast<int>(rowDatas.size()); ++i)
			{
				// 左上に配置
				// 余白を少し取るために、オフセットする
				constexpr Lattice2 IndexOffset = Lattice2(1, 1);
				const auto& rowData = rowDatas[i];
				textRenderer.data.SetTexts(Lattice2(0, i) + IndexOffset, rowData.text, rowData.color);
			}
		}
	};
}
