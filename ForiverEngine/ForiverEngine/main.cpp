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

	// 頂点は時計回り！！
	XMFLOAT3 vertices[] =
	{
		{ -1, -1, 0 }, // 左下
		{ -1, 1, 0 }, // 左上
		{ 1, -1, 0 }, // 右下
	};

	GraphicsBuffer vertexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, sizeof(vertices), true);
	if (!vertexBuffer) Throw(L"頂点バッファーの作成に失敗しました");
	if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer(vertexBuffer, vertices, sizeof(vertices)))
		Throw(L"頂点バッファーを GPU 側にコピーすることに失敗しました");

	// シェーダーをロード
	CompiledShaderObject shaderVS, shaderPS;
	{
		std::wstring errorMessage;

		shaderVS = D3D12Helper::CompileShaderFile(L"./shaders/BasicVS.hlsl", "VSMain", ShaderTargetVS, errorMessage);
		if (!shaderVS)
			Throw(errorMessage.c_str());

		shaderPS = D3D12Helper::CompileShaderFile(L"./shaders/BasicPS.hlsl", "PSMain", ShaderTargetPS, errorMessage);
		if (!shaderPS)
			Throw(errorMessage.c_str());
	}

	// 頂点レイアウトを作成
	std::vector<VertexLayout> vertexLayouts =
	{
		{ "POSITION", Format::RGBA_F32 },
	};

	PipelineState graphicsPipelineState = D3D12Helper::CreateGraphicsPipelineState(
		device, shaderVS, shaderPS, vertexLayouts, FillMode::Solid, CullMode::None);
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
		D3D12Helper::CommandSetRTAsOutputStage(commandList, backBufferRTV);
		D3D12Helper::CommandClearRT(commandList, backBufferRTV, { 1, 1, 0, 1 });
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
