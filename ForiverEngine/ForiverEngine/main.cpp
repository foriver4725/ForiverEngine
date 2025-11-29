#include "./headers/WindowHelper.h"
#include "./headers/D3D12ObjectFactory.h"

#include <Windows.h>

#if true
// テスト用
#include <string>
#include <functional>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#endif

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

const wchar_t* WindowClassName = L"DX12Sample";
const wchar_t* WindowTitle = L"DX12 テスト";

DEFINE_DEFAULT_WINDOW_PROCEDURE(WindowProcedure)

int WindowMain(hInstance)
{
	using namespace ForiverEngine;

#define Throw(Message) \
{ \
    WindowHelper::PopupErrorDialog(Message); \
    return -1; \
}



	if (!WindowHelper::InitializeWindowFromHInstance(hInstance, WindowProcedure, WindowClassName))
		Throw(L"ウィンドウの初期化に失敗しました");
	HWND hwnd = WindowHelper::CreateTheWindow(WindowClassName, WindowTitle, WindowWidth, WindowHeight);

#ifdef _DEBUG
	if (!D3D12ObjectFactory::EnableDebugLayer())
		Throw(L"DebugLayer の有効化に失敗しました");
#endif

	Factory factory = D3D12ObjectFactory::CreateFactory();
	if (!factory) Throw(L"Factory の作成に失敗しました");
	Device device = D3D12ObjectFactory::CreateDevice(factory);
	if (!device) Throw(L"Device の作成に失敗しました");

	CommandAllocator commandAllocater = D3D12ObjectFactory::CreateCommandAllocator(device);
	if (!commandAllocater) Throw(L"CommandAllocater の作成に失敗しました");
	CommandList commandList = D3D12ObjectFactory::CreateCommandList(device, commandAllocater);
	if (!commandList) Throw(L"CommandList の作成に失敗しました");
	CommandQueue commandQueue = D3D12ObjectFactory::CreateCommandQueue(device);
	if (!commandQueue) Throw(L"CommandQueue の作成に失敗しました");

	SwapChain swapChain = D3D12ObjectFactory::CreateSwapChain(factory, commandQueue, hwnd, WindowWidth, WindowHeight);
	if (!swapChain) Throw(L"SwapChain の作成に失敗しました");
	DescriptorHeap descriptorHeapRTV = D3D12ObjectFactory::CreateDescriptorHeapRTV(device);
	if (!descriptorHeapRTV) Throw(L"DescriptorHeap (RTV) の作成に失敗しました");
	if (!D3D12ObjectFactory::LinkDescriptorHeapRTVToSwapChain(device, descriptorHeapRTV, swapChain))
		Throw(L"DescriptorHeap (RTV) を SwapChain に関連付けることに失敗しました");



	// メッセージループ
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// 現在バックバッファにある RenderTarget を取得する
		DescriptorHeapHandle backBufferRTV = D3D12ObjectFactory::CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
			device, descriptorHeapRTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, swapChain->GetCurrentBackBufferIndex());

		if (!D3D12ObjectFactory::ClearCommandAllocatorAndList(commandAllocater, commandList))
			Throw(L"CommandAllocator, CommandList のクリアに失敗しました");

		D3D12ObjectFactory::CommandSetRTAsOutputStage(commandList, backBufferRTV);
		float clearColor[4] = { 1, 1, 0, 1 };
		D3D12ObjectFactory::CommandClearRT(commandList, backBufferRTV, clearColor);
		D3D12ObjectFactory::CommandClose(commandList);
		D3D12ObjectFactory::ExecuteCommands(commandQueue, commandList);

		Fence fence = D3D12ObjectFactory::CreateFence(device);
		D3D12ObjectFactory::WaitForGPUEventCompletion(fence, commandQueue);

		if (!D3D12ObjectFactory::ClearCommandAllocatorAndList(commandAllocater, commandList))
			Throw(L"CommandAllocator, CommandList のクリアに失敗しました");

		if (!D3D12ObjectFactory::Present(swapChain))
			Throw(L"画面のフリップに失敗しました");
	}

	return 0;
}
