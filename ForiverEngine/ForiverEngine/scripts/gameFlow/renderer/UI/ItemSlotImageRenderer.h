#pragma once

#include "scripts/gameFlow/IncludeInternal.h"

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
		static constexpr std::uint16_t ZOrder = 1;

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
			const RenderContext& renderContext, const Lattice2& windowSize,
			const Lattice2& position, const Lattice2& size, ImageType initType
		) :
			imageTypeToTexture
		{
			{ ImageType::Normal,   D3D12Utils::LoadTexture({ ImageTypeToFilePath.at(ImageType::Normal)   }) },
			{ ImageType::Selected, D3D12Utils::LoadTexture({ ImageTypeToFilePath.at(ImageType::Selected) }) },
		}
		{
			Base::Init(
				renderContext, windowSize,
				ImageTypeToFilePath.at(initType),
				position, size, ZOrder
			);
		}

		/// <summary>
		/// <para>画像の種類を変更する (通常 or 選択中)</para>
		/// <para>内部で GPU に再アップロードする</para>
		/// </summary>
		void ChangeImageType(const RenderContext& renderContext, ImageType newType)
		{
			Base::ReUploadTexture(renderContext, imageTypeToTexture.at(newType));
		}

	private:
		// 最初にロードして、キャッシュしておく
		const std::unordered_map<ImageType, Texture> imageTypeToTexture;
	};
}
