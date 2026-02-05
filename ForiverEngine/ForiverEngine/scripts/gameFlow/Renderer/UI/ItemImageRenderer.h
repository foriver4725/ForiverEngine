#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./AImageRenderer.h"
#include "scripts/gameFlow/Chunk.h"

namespace ForiverEngine
{
	/// <summary>
	/// アイテム画像のレンダラー (現在はブロック画像のみ設定できる)
	/// </summary>
	class ItemImageRenderer : public AImageRenderer
	{
	private:
		using Base = AImageRenderer;

	public:
		inline static const std::unordered_map<Block, std::string> BlockToFilePath =
		{
			{ Block::Air,     "assets/textures/ui/block/air.png"     },
			{ Block::Invalid, "assets/textures/ui/block/invalid.png" },
			{ Block::Grass,   "assets/textures/ui/block/grass.png"   },
			{ Block::Stone,   "assets/textures/ui/block/stone.png"   },
			{ Block::Dirt,    "assets/textures/ui/block/dirt.png"    },
			{ Block::Sand,    "assets/textures/ui/block/sand.png"    },
		};

		explicit ItemImageRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize,
			Block initType,
			const Lattice2& position, const Lattice2& drawSize // 描画サイズ (ピクセル単位)
		) :
			blockToTexture
		{
			{ Block::Air,     D3D12Utils::LoadTexture({ BlockToFilePath.at(Block::Air) })     },
			{ Block::Invalid, D3D12Utils::LoadTexture({ BlockToFilePath.at(Block::Invalid) }) },
			{ Block::Grass,   D3D12Utils::LoadTexture({ BlockToFilePath.at(Block::Grass) })   },
			{ Block::Stone,   D3D12Utils::LoadTexture({ BlockToFilePath.at(Block::Stone) })   },
			{ Block::Dirt,    D3D12Utils::LoadTexture({ BlockToFilePath.at(Block::Dirt) })    },
			{ Block::Sand,    D3D12Utils::LoadTexture({ BlockToFilePath.at(Block::Sand) })    },
		}
		{
			Base::Init(
				device, commandList, commandQueue, commandAllocator, windowSize,
				BlockToFilePath.at(initType), position, Vector2::Zero(), Vector2::One(), drawSize,
				IsDrawableBlock(initType)
			);
		}

		/// <summary>
		/// <para>画像の種類を変更する</para>
		/// <para>内部で GPU に再アップロードする</para>
		/// </summary>
		void ChangeImageType(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			Block newType
		)
		{
			// t1
			Base::ReUploadTexture(device, commandList, commandQueue, commandAllocator,
				blockToTexture.at(newType), ShaderRegister::t1);

			Base::SetDrawEnabled(IsDrawableBlock(newType));
		}

	private:
		// 最初にロードして、キャッシュしておく
		const std::unordered_map<Block, Texture> blockToTexture;

		// 「ちゃんとした」ブロックでないと、描画しない
		static constexpr bool IsDrawableBlock(Block block)
		{
			return block != Block::Invalid;
		}
	};
}
