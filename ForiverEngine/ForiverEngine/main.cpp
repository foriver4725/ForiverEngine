#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "./headers/D3D12ObjectFactory.h"

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

const wchar_t* WindowClassName = L"DX12Sample";
const wchar_t* WindowTitle = L"DX12 テスト";

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#undef CreateWindow
void CreateWindow(const wchar_t* className, const wchar_t* title, int width, int height);
void PopupErrorDialog(const wchar_t* message);

// WinMain() 内で使う
// エラーポップアップを出してから -1 を返す
#define ThrowAsDialogPopup(message) \
	do \
	{ \
		PopupErrorDialog(message); \
		return -1; \
	} while (0)

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	using namespace ForiverEngine;

	WNDCLASSW w = { };
	w.hInstance = hInstance;
	w.lpszClassName = WindowClassName;
	w.lpfnWndProc = WindowProcedure;

	// ウィンドウクラスの登録
	if (!RegisterClassW(&w))
		return -1;

	// ウィンドウの作成
	CreateWindow(WindowClassName, WindowTitle, WindowWidth, WindowHeight);

	// 基本オブジェクトの生成
	IDXGISwapChain4* _swapchain = nullptr;

	IDXGIFactory6* _dxgiFactory = D3D12ObjectFactory::CreateDXGIFactory();
	if (!_dxgiFactory)
		ThrowAsDialogPopup(L"DXGIFactory の作成に失敗しました");

	D3D_FEATURE_LEVEL featureLevel;
	ID3D12Device* _dev = D3D12ObjectFactory::CreateD3D12Device(_dxgiFactory, featureLevel);
	if (!_dev)
		ThrowAsDialogPopup(L"D3D12Device の作成に失敗しました");

	// コマンドアロケーター・コマンドリストを作成
	ID3D12CommandAllocator* _commandAllocater = nullptr;
	ID3D12GraphicsCommandList* _commandList = nullptr;
	if (_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocater)) != S_OK)
	{
		ThrowAsDialogPopup(L"CommandAllocater の作成に失敗しました");
	}
	if (_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocater, nullptr, IID_PPV_ARGS(&_commandList)) != S_OK)
	{
		ThrowAsDialogPopup(L"CommandList の作成に失敗しました");
	}

	// コマンドキューを作成
	ID3D12CommandQueue* _commandQueue = nullptr;
	{
		D3D12_COMMAND_QUEUE_DESC desc
		{
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストと合わせる
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, // 優先度は通常
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE, // 既定
			.NodeMask = 0, // アダプターが1つなので...
		};

		if (_dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue)) != S_OK)
		{
			PopupErrorDialog(L"CommandQueue の作成に失敗しました");
			return -1;
		}
	}

	// メッセージループ
	MSG msg = { };
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

void CreateWindow(const wchar_t* className, const wchar_t* title, int width, int height)
{
	CreateWindowW(className, title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
