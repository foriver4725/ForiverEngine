#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./AImageRenderer.h"

namespace ForiverEngine
{
	/// <summary>
	/// アイテムスロット画像1つ分のレンダラー
	/// </summary>
	class ItemSlotImageRenderer : public AImageRenderer
	{
	private:
		using Base = AImageRenderer;

	public:
		static constexpr int SlotCount = 6; // スロット数
		static constexpr int SlotSize = 128; // スロット1つ分のサイズ (正方形. NxN)

		explicit ItemSlotImageRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize,
			const Lattice2& position, const Lattice2& drawSize // 描画サイズ (ピクセル単位)
		)
		{
			Base::Init(
				device, commandList, commandQueue, commandAllocator, windowSize,
				"assets/textures/item_frame.png", position, Vector2::Zero(), Vector2::One(), drawSize
			);
		}
	};
}
