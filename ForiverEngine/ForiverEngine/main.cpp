#include "./headers/WindowHelper.h"
#include "./headers/D3D12ObjectFactory.h"

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
	{
		WindowHelper::PopupErrorDialog(L"ウィンドウの初期化に失敗しました。");
		return -1;
	}

	HWND hwnd = WindowHelper::CreateTheWindow(WindowClassName, WindowTitle, WindowWidth, WindowHeight);



	Factory factory = D3D12ObjectFactory::CreateFactory();
	if (!factory) { WindowHelper::PopupErrorDialog(L"Factory の作成に失敗しました。"); return -1; }

	Device device = D3D12ObjectFactory::CreateDevice(factory);
	if (!device) { WindowHelper::PopupErrorDialog(L"Device の作成に失敗しました。"); return -1; }

	CommandAllocator commandAllocater = D3D12ObjectFactory::CreateCommandAllocator(device);
	if (!commandAllocater) { WindowHelper::PopupErrorDialog(L"CommandAllocator の作成に失敗しました。"); return -1; }

	CommandList commandList = D3D12ObjectFactory::CreateCommandList(device, commandAllocater);
	if (!commandList) { WindowHelper::PopupErrorDialog(L"CommandList の作成に失敗しました。"); return -1; }

	CommandQueue commandQueue = D3D12ObjectFactory::CreateCommandQueue(device);
	if (!commandQueue) { WindowHelper::PopupErrorDialog(L"CommandQueue の作成に失敗しました。"); return -1; }

	SwapChain swapChain = D3D12ObjectFactory::CreateSwapChain(factory, commandQueue, hwnd, WindowWidth, WindowHeight);
	if (!swapChain) { WindowHelper::PopupErrorDialog(L"SwapChain の作成に失敗しました。"); return -1; }

	DescriptorHeap descriptorHeap = D3D12ObjectFactory::CreateDescriptorHeap(device);
	if (!descriptorHeap) { WindowHelper::PopupErrorDialog(L"DescriptorHeap の作成に失敗しました。"); return -1; }



	// メッセージループ
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
