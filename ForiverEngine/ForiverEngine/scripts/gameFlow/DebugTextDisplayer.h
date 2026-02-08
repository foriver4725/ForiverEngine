#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./PlayerController.h"
#include "./ChunksManager.h"
#include "./DebugFrameTimeStats.h"
#include "./Renderer/Include.h"

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

		// データを更新し、GPU にも反映させる
		void UpdateDataAsFold(const RenderContext& renderContext, TextRenderer& textRenderer)
		{
			rowDatas.clear();
			rowDatas.emplace_back("Press F1 to unfold debug info.", TextColor);

			ApplyDataToRenderer(rowDatas, textRenderer);
			textRenderer.UpdateDataAtGPU(renderContext);
		}

		// データを更新し、GPU にも反映させる
		void UpdateDataAsUnfold(
			const RenderContext& renderContext, TextRenderer& textRenderer,
			// 多くの処理で共通して使う
			const PlayerController& playerController, const ChunksManager& chunksManager,
			// 以下は個別の処理で使う
			const DebugFrameTimeStatsBreakdown& frameTimeStatsBreakdown, const DebugText::LookingBlockInfo& lookingBlockInfo
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
			rowDatas.emplace_back(DebugText::LookingBlock(lookingBlockInfo, chunksManager), TextColor);    // 2
			rowDatas.emplace_back(DebugText::ChunkIndex(playerController), TextColor);                     // 3
			rowDatas.emplace_back(DebugText::ChunkLocalPosition(playerController), TextColor);             // 4
			rowDatas.emplace_back(DebugText::DrawChunksRange(chunksManager), TextColor);                   // 5
			rowDatas.emplace_back(DebugText::CollisionRange(playerController), TextColor);                 // 6
			rowDatas.emplace_back(DebugText::FloorCeilHeight(playerController, chunksManager), TextColor); // 7

			ApplyDataToRenderer(rowDatas, textRenderer);
			textRenderer.UpdateDataAtGPU(renderContext);
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
