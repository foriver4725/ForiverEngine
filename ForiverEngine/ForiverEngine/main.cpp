#include "./headers/WindowHelper.h"
#include "./headers/D3D12Helper.h"

#include <Windows.h>

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

const wchar_t* WindowClassName = L"DX12Sample";
const wchar_t* WindowTitle = L"DX12 テスト";

DEFINE_DEFAULT_WINDOW_PROCEDURE(WindowProcedure)

int WindowMain(hInstance)
{
	using namespace ForiverEngine;

	if (!WindowHelper::InitializeWindowFromHInstance(hInstance, WindowProcedure, WindowClassName))
		Throw(L"ウィンドウの初期化に失敗しました");
	HWND hwnd = WindowHelper::CreateTheWindow(WindowClassName, WindowTitle, WindowWidth, WindowHeight);

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



	// メッセージループ
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// 現在バックバッファにある RenderTarget を取得する
		const int currentBackBufferIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);
		GraphicBuffer currentBackBuffer = D3D12Helper::GetGraphicBufferByIndex(swapChain, currentBackBufferIndex);
		if (!currentBackBuffer) Throw(L"現在バックにある GraphicBuffer を取得することに失敗しました");
		DescriptorHeapHandleAtCPU backBufferRTV = D3D12Helper::CreateDescriptorRTVHandleByIndex(
			device, descriptorHeapRTV, currentBackBufferIndex);

		D3D12Helper::InvokeResourceBarrierAsTransitionFromPresentToRenderTarget(commandList, currentBackBuffer);

		D3D12Helper::CommandSetRTAsOutputStage(commandList, backBufferRTV);
		float clearColor[4] = { 1, 1, 0, 1 };
		D3D12Helper::CommandClearRT(commandList, backBufferRTV, clearColor);

		D3D12Helper::InvokeResourceBarrierAsTransitionFromRenderTargetToPresent(commandList, currentBackBuffer);

		D3D12Helper::CommandClose(commandList);
		D3D12Helper::ExecuteCommands(commandQueue, commandList);

		Fence fence = D3D12Helper::CreateFence(device);
		D3D12Helper::WaitForGPUEventCompletion(fence, commandQueue);

		if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocater, commandList))
			Throw(L"CommandAllocator, CommandList のクリアに失敗しました");

		if (!D3D12Helper::Present(swapChain))
			Throw(L"画面のフリップに失敗しました");
	}

	return 0;
}
