#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./AOffscreenRenderer.h"

namespace ForiverEngine
{
	/// <summary>
	/// ポストプロセス用のレンダラー
	/// </summary>
	class PostProcessRenderer : public AOffscreenRenderer
	{
	private:
		using Base = AOffscreenRenderer;

		// b0
		struct alignas(256) CBData0
		{
			std::uint32_t WindowSize[2];
			float LimitLuminance;        // ピクセルがモデルの端にあると判断する輝度差の閾値 ([0.0, 1.0]. 小さいほどAAが多くかかる)
			float AAPower;               // アンチエイリアスの強さ (大きいほどAAが強くかかる)

			Color PointerColor;          // 画面中央のポインタの色
			std::uint32_t PointerLength; // 画面中央のポインタの長さ (ピクセル数. 奇数前提)
			std::uint32_t PointerWidth;  // 画面中央のポインタの太さ (ピクセル数. 奇数前提)
		};

	public:
		explicit PostProcessRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize
		)
		{
			// b0
			const CBData0 cbData0 =
			{
				.WindowSize = { static_cast<std::uint32_t>(windowSize.x), static_cast<std::uint32_t>(windowSize.y) },
				.LimitLuminance = 0.5f,
				.AAPower = 8.0f,

				.PointerColor = Color::Black(),
				.PointerLength = 21,
				.PointerWidth = 3,
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(device, cbData0);

			Base::Init(device, windowSize, { cb0 }, {}, "./shaders/PP.hlsl");
		}
	};
}
