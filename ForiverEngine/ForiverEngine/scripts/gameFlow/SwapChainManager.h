#pragma once

#include "scripts/gameFlow/IncludeInternal.h"

namespace ForiverEngine
{
	class SwapChainManager
	{
	public:
		explicit SwapChainManager(
			const Factory& factory, const Device& device, const CommandQueue& commandQueue,
			HWND hwnd, const Lattice2& windowSize
		)
		{
			swapChain = D3D12Helper::CreateSwapChain(factory, commandQueue, hwnd, windowSize);
			if (!swapChain)
				ShowError(L"SwapChain の作成に失敗しました");

			std::tie(rtGetter, rtvGetter) = D3D12Utils::InitRTV(device, swapChain, Format::RGBA_U8_01);
		}

		/// <summary>
		/// 現在バックにある RT を RenderTargetContext として返す
		/// </summary>
		RenderTargetContext GetCurrentRenderTargetContext(const ViewportScissorRect& viewportScissorRect) const
		{
			const int currentBackRTIndex = D3D12Helper::GetCurrentBackRTIndex(swapChain);

			const GraphicsBuffer currentBackRT = rtGetter(currentBackRTIndex);
			if (!currentBackRT)
				ShowError(L"現在のバックレンダーターゲットの取得に失敗しました");
			const DescriptorHandleAtCPU currentBackRTV = rtvGetter(currentBackRTIndex);

			return RenderTargetContext
			{
				.rt = currentBackRT,
				.rtv = currentBackRTV,
				.viewportScissorRect = viewportScissorRect,
			};
		}

		/// <summary>
		/// 画面のフリップを行う. 描画の一番最後に、必ず呼び出す必要がある
		/// </summary>
		void Present() const
		{
			if (!D3D12Helper::Present(swapChain))
				ShowError(L"画面のフリップに失敗しました");
		}

	private:
		SwapChain swapChain;

		std::function<GraphicsBuffer(int)> rtGetter;
		std::function<DescriptorHandleAtCPU(int)> rtvGetter;
	};
}
