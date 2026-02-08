#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./Chunk.h"
#include "./Renderer/Include.h"

namespace ForiverEngine
{
	// TODO: 依存関係が汚い! Renderer の参照は持ちたくない
	class ItemSlotManager
	{
	private:
		using SlotImageType = ItemSlotImageRenderer::ImageType;

	public:
		static constexpr int SlotCount = 6;  // スロット数
		static constexpr int SlotSize = 128; // スロット画像1つ分のサイズ (正方形. NxN)
		static constexpr int ItemSize = 96;  // アイテム画像1つ分のサイズ (正方形. NxN)

		// それぞれのアイテムスロットに、どのアイテムが入っているか
		// 現在は画像の種類が変化することはないので、固定配列で良い
		static constexpr Block SlotItems[SlotCount] =
		{
			Block::Grass,
			Block::Stone,
			Block::Dirt,
			Block::Sand,

			// ブロックが無いことを表す
			Block::Invalid,
			Block::Invalid,
		};

		// 上から順に入力を確認していって、最初に検知した入力に応じて、選択中スロットを変更する
		struct SelectInputs
		{
			bool select1;     // スロット1を選択   (押された瞬間だけ true)
			bool select2;     // スロット2を選択   (押された瞬間だけ true)
			bool select3;     // スロット3を選択   (押された瞬間だけ true)
			bool select4;     // スロット4を選択   (押された瞬間だけ true)
			bool select5;     // スロット5を選択   (押された瞬間だけ true)
			bool select6;     // スロット6を選択   (押された瞬間だけ true)

			bool selectLeft;  // スロットを左に移動 (押された瞬間だけ true)
			bool selectRight; // スロットを右に移動 (押された瞬間だけ true)
		};

		ItemSlotManager(const RenderContext& renderContext, const Lattice2& windowSize, int initSelectingIndex = 0) :
			slotImageRenderers(CreateSlotImageRenderers(renderContext, windowSize)),
			itemImageRenderers(CreateItemImageRenderers(renderContext, windowSize))
		{
			selectingIndex = initSelectingIndex;
			ChangeSlotImage(renderContext, initSelectingIndex, SlotImageType::Selected);
		}

		/// <summary>
		/// <para>入力情報に基づいて、選択中スロットを更新する (入力が無ければ何もしない)</para>
		/// <para>選択中スロットが変化した場合は、スロット画像の見た目も更新する (強調表示など)</para>
		/// </summary>
		void UpdateSelectingSlotByInput(const RenderContext& renderContext, const SelectInputs& inputs)
		{
			int newIndex = selectingIndex;

			/**/ if (inputs.select1)     newIndex = 0;
			else if (inputs.select2)     newIndex = 1;
			else if (inputs.select3)     newIndex = 2;
			else if (inputs.select4)     newIndex = 3;
			else if (inputs.select5)     newIndex = 4;
			else if (inputs.select6)     newIndex = 5;

			else if (inputs.selectLeft)  newIndex = (selectingIndex - 1 + SlotCount) % SlotCount;
			else if (inputs.selectRight) newIndex = (selectingIndex + 1) % SlotCount;

			// 入力が無いので何もせず、以降の処理もスキップして良い
			else return;

			UpdateSelectingSlot(renderContext, newIndex);
		}

		/// <summary>
		/// <para>選択中スロットを、指定したインデックスに更新する</para>
		/// <para>選択中スロットが変化した場合は、スロット画像の見た目も更新する (強調表示など)</para>
		/// <para>選択中スロットが変化しなければ、何もしない</para>
		/// </summary>
		void UpdateSelectingSlot(const RenderContext& renderContext, int newIndex)
		{
			// 選択が変化しないので、何もしない
			if (selectingIndex == newIndex)
				return;

			const int prevIndex = selectingIndex;
			selectingIndex = newIndex;

			// スロット画像を更新する (選択しているスロットだけ強調表示する)
			ChangeSlotImage(renderContext, prevIndex, SlotImageType::Normal);
			ChangeSlotImage(renderContext, selectingIndex, SlotImageType::Selected);
		}

		/// <summary>
		/// <para>描画に必要なレンダラー群を、1つの配列にまとめて返す</para>
		/// <para>返された配列の順番で描画すれば良い</para>
		/// </summary>
		std::vector<const AOffscreenRenderer*> PackRenderers() const
		{
			std::vector<const AOffscreenRenderer*> renderers;
			renderers.reserve(SlotCount * 2);

			for (const auto& slot : slotImageRenderers)
				renderers.push_back(slot.get());
			for (const auto& item : itemImageRenderers)
				renderers.push_back(item.get());

			return renderers;
		}

		int GetSelectingIndex() const
		{
			return selectingIndex;
		}

	private:
		// 描画に必要な Renderer 群
		const std::vector<std::unique_ptr<AOffscreenRenderer>> slotImageRenderers;
		const std::vector<std::unique_ptr<AOffscreenRenderer>> itemImageRenderers;

		int selectingIndex = 0; // 現在選択中のスロットのインデックス [0, SlotCount)

		// 画面上の位置 (ピクセル単位)
		// 隙間なしで、画面下中央あたりに並べる
		static constexpr Lattice2 GetSlotPosition(const Lattice2& windowSize, int slotIndex)
		{
			constexpr int MarginBottom = 20; // 画面下からの余白 (ピクセル単位)

			return Lattice2(
				(windowSize.x - SlotSize * (SlotCount - 1)) / 2 + slotIndex * SlotSize,
				windowSize.y - MarginBottom - SlotSize / 2
			);
		}

		// slotPositions を参照してしまっているが、const だから大丈夫だと思う
		static std::vector<std::unique_ptr<AOffscreenRenderer>>
			CreateSlotImageRenderers(const RenderContext& renderContext, const Lattice2& windowSize)
		{
			std::vector<std::unique_ptr<AOffscreenRenderer>> renderers;
			renderers.reserve(SlotCount);

			for (int i = 0; i < SlotCount; ++i)
			{
				renderers.push_back(std::make_unique<ItemSlotImageRenderer>(
					renderContext, windowSize,
					GetSlotPosition(windowSize, i), Lattice2(SlotSize, SlotSize),
					SlotImageType::Normal
				));
			}

			return renderers;
		}

		// slotPositions を参照してしまっているが、const だから大丈夫だと思う
		static std::vector<std::unique_ptr<AOffscreenRenderer>>
			CreateItemImageRenderers(const RenderContext& renderContext, const Lattice2& windowSize)
		{
			std::vector<std::unique_ptr<AOffscreenRenderer>> renderers;
			renderers.reserve(SlotCount);

			for (int i = 0; i < SlotCount; ++i)
			{
				renderers.push_back(std::make_unique<ItemImageRenderer>(
					renderContext, windowSize,
					GetSlotPosition(windowSize, i), Lattice2(ItemSize, ItemSize),
					SlotItems[i]
				));
			}

			return renderers;
		}

		void ChangeSlotImage(const RenderContext& renderContext, int index, SlotImageType newType)
		{
			ItemSlotImageRenderer* slotImageRenderer = dynamic_cast<ItemSlotImageRenderer*>(slotImageRenderers[index].get());
			slotImageRenderer->ChangeImageType(renderContext, newType);
		}
	};
}
