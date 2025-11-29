#include <Windows.h>

#include "./headers/D3D12ObjectFactory.h"

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

const wchar_t* WindowClassName = L"DX12Sample";
const wchar_t* WindowTitle = L"DX12 テスト";

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#undef CreateWindow
HWND CreateWindow(const wchar_t* className, const wchar_t* title, int width, int height);
void PopupErrorDialog(const wchar_t* message);


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	using namespace ForiverEngine;

	WNDCLASSW w = {};
	w.hInstance = hInstance;
	w.lpszClassName = WindowClassName;
	w.lpfnWndProc = WindowProcedure;

	// ウィンドウクラスの登録
	if (!RegisterClassW(&w))
		return -1;

	// ウィンドウの作成
	HWND hwnd = CreateWindow(WindowClassName, WindowTitle, WindowWidth, WindowHeight);

	Factory factory = D3D12ObjectFactory::CreateFactory();
	if (!factory.IsValid()) { PopupErrorDialog(L"Factory の作成に失敗しました。"); return -1; }

	Device device = D3D12ObjectFactory::CreateDevice(factory);
	if (!device.IsValid()) { PopupErrorDialog(L"Device の作成に失敗しました。"); return -1; }

	CommandAllocator commandAllocater = D3D12ObjectFactory::CreateCommandAllocator(device);
	if (!commandAllocater.IsValid()) { PopupErrorDialog(L"CommandAllocator の作成に失敗しました。"); return -1; }

	CommandList commandList = D3D12ObjectFactory::CreateCommandList(device, commandAllocater);
	if (!commandList.IsValid()) { PopupErrorDialog(L"CommandList の作成に失敗しました。"); return -1; }

	CommandQueue commandQueue = D3D12ObjectFactory::CreateCommandQueue(device);
	if (!commandQueue.IsValid()) { PopupErrorDialog(L"CommandQueue の作成に失敗しました。"); return -1; }

	SwapChain swapChain = D3D12ObjectFactory::CreateSwapChain(factory, commandQueue, hwnd, WindowWidth, WindowHeight);
	if (!swapChain.IsValid()) { PopupErrorDialog(L"SwapChain の作成に失敗しました。"); return -1; }

	// メッセージループ
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

// --------------------------------------------------

// エラーポップアップを出す
void PopupErrorDialog(const wchar_t* message)
{
	MessageBox(nullptr, message, L"error", MB_OK | MB_ICONERROR);
}

HWND CreateWindow(const wchar_t* className, const wchar_t* title, int width, int height)
{
	return CreateWindowW(className, title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, nullptr, nullptr);
}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); // OSに対して「もうこのアプリは終わる」と伝える
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam); // デフォルトの処理を行う
}
