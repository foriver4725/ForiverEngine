#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "scripts/gameFlow/Renderer/Context/Include.h"
#include "scripts/gameFlow/Renderer/Offscreen/AOffscreenRenderer.h"

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
		};

	public:
		explicit PostProcessRenderer(const RenderContext& renderContext, const Lattice2& windowSize)
		{
			// b0
			const CBData0 cbData0 =
			{
				.WindowSize = { static_cast<std::uint32_t>(windowSize.x), static_cast<std::uint32_t>(windowSize.y) },
				.LimitLuminance = 0.5f,
				.AAPower = 8.0f,
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(renderContext.device, cbData0);

			Base::Init(
				renderContext, windowSize,
				{ cb0 }, {}, D3D12Utils::GetShaderFilePath("PP"));
		}
	};
}
