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

		inline static const std::string NormalImageFilePath = "assets/textures/ui/item_frame.png";
		inline static const std::string SelectedImageFilePath = "assets/textures/ui/item_frame_selected.png";

		enum class ImageType : std::uint8_t
		{
			Normal,
			Selected,
		};

		explicit ItemSlotImageRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize,
			ImageType initType,
			const Lattice2& position, const Lattice2& drawSize // 描画サイズ (ピクセル単位)
		)
		{
			typeToTextureMap =
			{
				{ ImageType::Normal, D3D12Utils::LoadTexture({ NormalImageFilePath }) },
				{ ImageType::Selected, D3D12Utils::LoadTexture({ SelectedImageFilePath }) },
			};

			Base::Init(
				device, commandList, commandQueue, commandAllocator, windowSize,
				GetImageFilePath(initType), position, Vector2::Zero(), Vector2::One(), drawSize
			);
		}

		/// <summary>
		/// <para>画像の種類を変更する (通常 or 選択中)</para>
		/// <para>内部で GPU に再アップロードする</para>
		/// </summary>
		void ChangeImageType(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			ImageType newType
		)
		{
			// t1
			Base::ReUploadTexture(device, commandList, commandQueue, commandAllocator,
				typeToTextureMap[newType], ShaderRegister::t1);
		}

	private:
		// 最初にロードして、キャッシュしておく
		std::unordered_map<ImageType, Texture> typeToTextureMap;

		static const std::string& GetImageFilePath(ImageType type)
		{
			switch (type)
			{
			case ImageType::Normal:   return NormalImageFilePath;
			case ImageType::Selected: return SelectedImageFilePath;
			default:                  return NormalImageFilePath;
			}
		}
	};
}
