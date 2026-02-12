#pragma once

#include "scripts/gameFlow/IncludeInternal.h"
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
		static constexpr std::uint16_t ZOrder = 0;

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
			const RenderContext& renderContext, const Lattice2& windowSize,
			const Lattice2& position, const Lattice2& size, Block initType
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
				renderContext, windowSize,
				BlockToFilePath.at(initType),
				position, size, ZOrder, IsDrawableBlock(initType)
			);
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
