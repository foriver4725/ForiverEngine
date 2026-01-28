#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "../AOffscreenRenderer.h"

namespace ForiverEngine
{
	/// <summary>
	/// 1枚の画像について、位置やスケールを指定して描画するレンダラー
	/// </summary>
	class AImageRenderer : public AOffscreenRenderer
	{
	private:
		using Base = AOffscreenRenderer;

		// b0
		struct alignas(256) CBData0
		{
			std::uint32_t TextureSize[2]; // 画像サイズ
			Lattice2 Position;
			Vector2 Scale;
		};

	public:

		void Init(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize,
			const std::string& imageFilePath, const Lattice2& position, const Vector2& scale
		)
		{
			// t1 (画像テクスチャ)
			const Texture sr1Metadata = D3D12Utils::LoadTexture({ imageFilePath });
			const GraphicsBuffer sr1 = D3D12Utils::InitSR(device, commandList, commandQueue, commandAllocator, sr1Metadata);

			// b0
			const CBData0 cbData0 =
			{
				.TextureSize = { static_cast<std::uint32_t>(sr1Metadata.size.x), static_cast<std::uint32_t>(sr1Metadata.size.y) },
				.Position = position,
				.Scale = scale,
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(device, cbData0);

			Base::Init(device, windowSize, { cb0 }, { { sr1, sr1Metadata } }, D3D12Utils::GetShaderFilePath("Image"));
		}
	};
}
