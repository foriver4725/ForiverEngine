#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "../AOffscreenRenderer.h"

namespace ForiverEngine
{
	/// <summary>
	/// ポインタ(画面中央にあるクロスヘア)のレンダラー
	/// </summary>
	class PointerImageRenderer : public AOffscreenRenderer
	{
	private:
		using Base = AOffscreenRenderer;

		// b0
		struct alignas(256) CBData0
		{
			std::uint32_t PointerTextureSize[2]; // ポインタテクスチャのサイズ (ピクセル単位)
			Lattice2 PointerPosition;            // 画面上の位置 (ピクセル単位)
			Vector2 PointerScale;                // 画面上の実寸では、テクスチャサイズの何倍であるか
		};

	public:

		explicit PointerImageRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize
		)
		{
			// t1 (ポインタテクスチャ)
			const Texture sr1Metadata = D3D12Utils::LoadTexture({ "assets/textures/pointer.png" });
			const GraphicsBuffer sr1 = D3D12Utils::InitSR(device, commandList, commandQueue, commandAllocator, sr1Metadata);

			// b0
			const CBData0 cbData0 =
			{
				.PointerTextureSize = { static_cast<std::uint32_t>(sr1Metadata.size.x), static_cast<std::uint32_t>(sr1Metadata.size.y) },
				.PointerPosition = windowSize / 2,
				.PointerScale = Vector2::One() * 3.0f,
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(device, cbData0);

			Base::Init(device, windowSize, { cb0 }, { { sr1, sr1Metadata } }, D3D12Utils::GetShaderFilePath("Pointer"));
		}
	};
}
