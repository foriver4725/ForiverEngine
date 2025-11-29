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

#define Throw(Message) \
{ \
    WindowHelper::PopupErrorDialog(Message); \
    return -1; \
}

#define CheckAndThrow(Pointer) if (!(Pointer)) Throw(L#Pointer L" が nullptr です");



	if (!WindowHelper::InitializeWindowFromHInstance(hInstance, WindowProcedure, WindowClassName))
		Throw(L"ウィンドウの初期化に失敗しました");
	HWND hwnd = WindowHelper::CreateTheWindow(WindowClassName, WindowTitle, WindowWidth, WindowHeight);

	Factory factory = D3D12ObjectFactory::CreateFactory();	CheckAndThrow(factory);
	Device device = D3D12ObjectFactory::CreateDevice(factory);	CheckAndThrow(device);
	CommandAllocator commandAllocater = D3D12ObjectFactory::CreateCommandAllocator(device);	CheckAndThrow(commandAllocater);
	CommandList commandList = D3D12ObjectFactory::CreateCommandList(device, commandAllocater);	CheckAndThrow(commandList);
	CommandQueue commandQueue = D3D12ObjectFactory::CreateCommandQueue(device);	CheckAndThrow(commandQueue);
	SwapChain swapChain = D3D12ObjectFactory::CreateSwapChain(factory, commandQueue, hwnd, WindowWidth, WindowHeight);	CheckAndThrow(swapChain);
	DescriptorHeap descriptorHeap = D3D12ObjectFactory::CreateDescriptorHeap(device);	CheckAndThrow(descriptorHeap);

	// メッセージループ
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
