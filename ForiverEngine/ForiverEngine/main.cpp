//#define ENABLE_CUI_CONSOLE

#include "./headers/D3D12BasicFlow.h"
#include "./headers/Math.h"

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

BEGIN_INITIALIZE(L"DX12Sample", L"DX12 テスト", hwnd, WindowWidth, WindowHeight);
{
	using namespace ForiverEngine;
	using namespace DirectX;

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

	// 頂点は時計回り！！
	const std::vector<XMFLOAT4> vertices =
	{
		{ -0.4f, -0.7f, 0, 1 },
		{ -0.4f, 0.7f, 0, 1 },
		{ 0.4f, -0.7f, 0, 1 },
		{ 0.4f, 0.7f, 0, 1 },
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
	Blob shaderVS, shaderPS;
	{
		std::wstring errorMessage;
		if (!D3D12Helper::CompileShaderFile_VS_PS(
			L"./shaders/Basic.hlsl",
			"VSMain", "PSMain",
			shaderVS, shaderPS,
			errorMessage
		))
			ShowError(errorMessage.c_str());
	}

	// 頂点レイアウトを作成
	std::vector<VertexLayout> vertexLayouts =
	{
		{ "POSITION", Format::RGBA_F32 },
	};

	RootSignature rootSignature;
	{
		std::wstring errorMessage;
		rootSignature = D3D12Helper::CreateRootSignature(device, errorMessage);
		if (!rootSignature)
			ShowError(errorMessage.c_str());
	}
	PipelineState graphicsPipelineState = D3D12Helper::CreateGraphicsPipelineState(
		device, rootSignature, shaderVS, shaderPS, vertexLayouts, FillMode::Solid, CullMode::None);
	if (!graphicsPipelineState) ShowError(L"GraphicsPipelineState の作成に失敗しました");

	BEGIN_MESSAGE_LOOP;
	{
		// 現在バックバッファにある RenderTarget を取得する
		const int currentBackBufferIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);
		GraphicsBuffer currentBackBuffer = D3D12Helper::GetBufferByIndex(swapChain, currentBackBufferIndex);
		if (!currentBackBuffer) ShowError(L"現在バックにある GraphicsBuffer を取得することに失敗しました");
		DescriptorHeapHandleAtCPU backBufferRTV = D3D12Helper::CreateDescriptorRTVHandleByIndex(
			device, descriptorHeapRTV, currentBackBufferIndex);

		D3D12Helper::InvokeResourceBarrierAsTransitionFromPresentToRenderTarget(commandList, currentBackBuffer);
		{
			D3D12Helper::CommandSetRTAsOutputStage(commandList, backBufferRTV);
			D3D12Helper::CommandClearRT(commandList, backBufferRTV, { 0, 0, 0, 1 });
			D3D12Helper::CommandSetGraphicsPipelineState(commandList, graphicsPipelineState);
			D3D12Helper::CommandSetRootSignature(commandList, rootSignature);
			D3D12Helper::CommandRSSetViewportAndScissorRect(commandList, viewportScissorRect);
			D3D12Helper::CommandIASetTopologyAsTriangleList(commandList);
			D3D12Helper::CommandIASetVertexBuffer(commandList, { vertexBufferView });
			D3D12Helper::CommandIASetIndexBuffer(commandList, indexBufferView);
			D3D12Helper::CommandDrawIndexedInstanced(commandList, indices.size());
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
