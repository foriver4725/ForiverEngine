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
		enum class ImageType : std::uint8_t
		{
			Normal,
			Selected,
		};

		inline static const std::unordered_map<ImageType, std::string> ImageTypeToFilePath =
		{
			{ ImageType::Normal,   "assets/textures/ui/item_frame.png"          },
			{ ImageType::Selected, "assets/textures/ui/item_frame_selected.png" },
		};

		explicit ItemSlotImageRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize,
			ImageType initType,
			const Lattice2& position, const Lattice2& drawSize // 描画サイズ (ピクセル単位)
		) :
			imageTypeToTexture
		{
			{ ImageType::Normal,   D3D12Utils::LoadTexture({ ImageTypeToFilePath.at(ImageType::Normal)   }) },
			{ ImageType::Selected, D3D12Utils::LoadTexture({ ImageTypeToFilePath.at(ImageType::Selected) }) },
		}
		{
			Base::Init(
				device, commandList, commandQueue, commandAllocator, windowSize,
				ImageTypeToFilePath.at(initType), position, Vector2::Zero(), Vector2::One(), drawSize,
				true
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
				imageTypeToTexture.at(newType), ShaderRegister::t1);
		}

	private:
		// 最初にロードして、キャッシュしておく
		const std::unordered_map<ImageType, Texture> imageTypeToTexture;
	};
}
