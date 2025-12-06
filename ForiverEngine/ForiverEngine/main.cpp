//#define ENABLE_CUI_CONSOLE

#include "./headers/WindowHelper.h"
#include "./headers/D3D12Helper.h"
#include "./headers/Math.h"

#include <Windows.h>
#include <DirectXMath.h>

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

BEGIN_INITIALIZE(L"DX12Sample", L"DX12 テスト", hwnd, WindowWidth, WindowHeight);
{
	using namespace ForiverEngine;
	using namespace DirectX;

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		Throw(L"DebugLayer の有効化に失敗しました");
#endif

	Factory factory = D3D12Helper::CreateFactory();
	if (!factory) Throw(L"Factory の作成に失敗しました");
	Device device = D3D12Helper::CreateDevice(factory);
	if (!device) Throw(L"Device の作成に失敗しました");

	CommandAllocator commandAllocater = D3D12Helper::CreateCommandAllocator(device);
	if (!commandAllocater) Throw(L"CommandAllocater の作成に失敗しました");
	CommandList commandList = D3D12Helper::CreateCommandList(device, commandAllocater);
	if (!commandList) Throw(L"CommandList の作成に失敗しました");
	CommandQueue commandQueue = D3D12Helper::CreateCommandQueue(device);
	if (!commandQueue) Throw(L"CommandQueue の作成に失敗しました");

	SwapChain swapChain = D3D12Helper::CreateSwapChain(factory, commandQueue, hwnd, WindowWidth, WindowHeight);
	if (!swapChain) Throw(L"SwapChain の作成に失敗しました");
	DescriptorHeap descriptorHeapRTV = D3D12Helper::CreateDescriptorHeapRTV(device);
	if (!descriptorHeapRTV) Throw(L"DescriptorHeap (RTV) の作成に失敗しました");
	if (!D3D12Helper::LinkDescriptorHeapRTVToSwapChain(device, descriptorHeapRTV, swapChain))
		Throw(L"DescriptorHeap (RTV) を SwapChain に関連付けることに失敗しました");

	// ビューポートとシザー矩形
	const ViewportScissorRect viewportScissorRect =
	{
		.minX = 0, .maxX = WindowWidth,
		.minY = 0, .maxY = WindowHeight,
	};

	// 頂点は時計回り！！
	XMFLOAT4 vertices[] =
	{
		{ -0.4f, -0.7f, 0, 1 },
		{ -0.4f, 0.7f, 0, 1 },
		{ 0.4f, -0.7f, 0, 1 },
		{ 0.4f, 0.7f, 0, 1 },
	};

	// 頂点インデックス
	std::uint16_t indices[] =
	{
		0, 1, 2,
		2, 1, 3,
	};

	GraphicsBuffer vertexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, sizeof(vertices), true);
	if (!vertexBuffer) Throw(L"頂点バッファーの作成に失敗しました");
	if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer(vertexBuffer, static_cast<void*>(vertices), sizeof(vertices)))
		Throw(L"頂点バッファーを GPU 側にコピーすることに失敗しました");
	VertexBufferView vertexBufferView = D3D12Helper::CreateVertexBufferView(vertexBuffer, sizeof(vertices), sizeof(vertices[0]));

	GraphicsBuffer indexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, sizeof(indices), true);
	if (!indexBuffer) Throw(L"インデックスバッファーの作成に失敗しました");
	if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer(indexBuffer, static_cast<void*>(indices), sizeof(indices)))
		Throw(L"インデックスバッファーを GPU 側にコピーすることに失敗しました");
	IndexBufferView indexBufferView = D3D12Helper::CreateIndexBufferView(indexBuffer, sizeof(indices), Format::R_U16);

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
			Throw(errorMessage.c_str());
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
			Throw(errorMessage.c_str());
	}
	PipelineState graphicsPipelineState = D3D12Helper::CreateGraphicsPipelineState(
		device, rootSignature, shaderVS, shaderPS, vertexLayouts, FillMode::Solid, CullMode::None);
	if (!graphicsPipelineState) Throw(L"GraphicsPipelineState の作成に失敗しました");

	BEGIN_MESSAGE_LOOP;
	{
		// 現在バックバッファにある RenderTarget を取得する
		const int currentBackBufferIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);
		GraphicsBuffer currentBackBuffer = D3D12Helper::GetBufferByIndex(swapChain, currentBackBufferIndex);
		if (!currentBackBuffer) Throw(L"現在バックにある GraphicsBuffer を取得することに失敗しました");
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
			D3D12Helper::CommandDrawIndexedInstanced(commandList, sizeof(indices) / sizeof(indices[0]));
		}
		D3D12Helper::InvokeResourceBarrierAsTransitionFromRenderTargetToPresent(commandList, currentBackBuffer);
		D3D12Helper::CommandClose(commandList);
		D3D12Helper::ExecuteCommands(commandQueue, commandList);

		D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue);

		if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocater, commandList))
			Throw(L"CommandAllocator, CommandList のクリアに失敗しました");

		if (!D3D12Helper::Present(swapChain))
			Throw(L"画面のフリップに失敗しました");
	}
	END_MESSAGE_LOOP;
}
END_INITIALIZE(0);
