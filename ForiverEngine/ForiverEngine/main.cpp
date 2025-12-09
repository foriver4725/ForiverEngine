//#define ENABLE_CUI_CONSOLE

#include "./headers/D3D12BasicFlow.h"
#include "./headers/Math.h"

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

BEGIN_INITIALIZE(L"DX12Sample", L"DX12 テスト", hwnd, WindowWidth, WindowHeight);
{
	using namespace ForiverEngine;

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	const auto [factory, device, commandAllocater, commandList, commandQueue, swapChain, descriptorHeapRTV]
		= D3D12BasicFlow::CreateStandardObjects(hwnd, WindowWidth, WindowHeight);

	// ビューポートとシザー矩形
	const ViewportScissorRect viewportScissorRect =
	{
		.minX = 0, .maxX = WindowWidth,
		.minY = 0, .maxY = WindowHeight,
	};

	const std::vector<VertexData> vertices =
	{
		{{ -0.4f, -0.7f, 0, 1 }, {0.0f, 1.0f}},
		{{ -0.4f, 0.7f, 0, 1 },  {0.0f, 0.0f}},
		{{ 0.4f, -0.7f, 0, 1 },  {1.0f, 1.0f}},
		{{ 0.4f, 0.7f, 0, 1 },   {1.0f, 0.0f}},
	};

	// 頂点インデックス
	const std::vector<std::uint16_t> indices =
	{
		0, 1, 2,
		2, 1, 3,
	};

	const auto [vertexBufferView, indexBufferView]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, vertices, indices);

	// シェーダーをロード
	const auto [shaderVS, shaderPS]
		= D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");

	// 頂点レイアウトを作成
	const std::vector<VertexLayout> vertexLayouts =
	{
		{ "POSITION", Format::RGBA_F32 },
	};

	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(device, shaderVS, shaderPS, vertexLayouts, FillMode::Solid, CullMode::None);

	BEGIN_MESSAGE_LOOP;
	{
		const auto [currentBackBuffer, currentBackBufferRTV]
			= D3D12BasicFlow::GetCurrentBackBufferAndCreateView(device, swapChain, descriptorHeapRTV);

		D3D12Helper::InvokeResourceBarrierAsTransitionFromPresentToRenderTarget(commandList, currentBackBuffer);
		{
			D3D12Helper::CommandSetRTAsOutputStage(commandList, currentBackBufferRTV);
			D3D12Helper::CommandClearRT(commandList, currentBackBufferRTV, { 0, 0, 0, 1 });
			D3D12Helper::CommandSetGraphicsPipelineState(commandList, graphicsPipelineState);
			D3D12Helper::CommandSetRootSignature(commandList, rootSignature);
			D3D12Helper::CommandRSSetViewportAndScissorRect(commandList, viewportScissorRect);
			D3D12Helper::CommandIASetTopologyAsTriangleList(commandList);
			D3D12Helper::CommandIASetVertexBuffer(commandList, { vertexBufferView });
			D3D12Helper::CommandIASetIndexBuffer(commandList, indexBufferView);
			D3D12Helper::CommandDrawIndexedInstanced(commandList, static_cast<int>(indices.size()));
		}
		D3D12Helper::InvokeResourceBarrierAsTransitionFromRenderTargetToPresent(commandList, currentBackBuffer);
		D3D12Helper::CommandClose(commandList);
		D3D12Helper::ExecuteCommands(commandQueue, commandList);

		D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue);

		if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocater, commandList))
			ShowError(L"CommandAllocator, CommandList のクリアに失敗しました");

		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_MESSAGE_LOOP;
}
END_INITIALIZE(0);
