#include <Windows.h>
#include <vector>
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

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

#define window_width 960
#define window_height 540

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	WNDCLASSW w = { };
	w.hInstance = hInstance;
	w.lpszClassName = L"DX12Sample";
	w.lpfnWndProc = WindowProcedure;

	// ウィンドウクラスの登録
	if (!RegisterClassW(&w))
		return -1;

	// ウィンドウの作成
	CreateWindowW(w.lpszClassName, L"DX12 テスト", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, nullptr, nullptr, nullptr, nullptr);

	// 基本オブジェクトの生成
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	// DXGIFactory を作成
	if (CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)) != S_OK)
	{
		MessageBox(nullptr, L"DXGIFactory の作成に失敗しました", L"error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// 利用可能なアダプターを取得 (グラボが複数刺さっている場合)
	std::vector<IDXGIAdapter*> adapters;
	{
		IDXGIAdapter* tmpAdapter = nullptr;
		for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			adapters.push_back(tmpAdapter);
		}
	}
	IDXGIAdapter* availableAdapter = nullptr;
	for (auto adapter : adapters)
	{
		DXGI_ADAPTER_DESC desc = {};
		adapter->GetDesc(&desc);
		std::wstring strDesc = desc.Description;

		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			availableAdapter = adapter;
			break;
		}
	}

	// ID3D12Device を作成
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL featureLevel;
	for (auto level : levels)
	{
		if (D3D12CreateDevice(availableAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = level;
			break;
		}
	}

	// コマンドアロケーター・コマンドリストを作成
	ID3D12CommandAllocator* _commandAllocater = nullptr;
	ID3D12GraphicsCommandList* _commandList = nullptr;
	if (_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocater)) != S_OK)
	{
		MessageBox(nullptr, L"CommandAllocater の作成に失敗しました", L"error", MB_OK | MB_ICONERROR);
		return -1;
	}
	if (_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocater, nullptr, IID_PPV_ARGS(&_commandList)) != S_OK)
	{
		MessageBox(nullptr, L"CommandList の作成に失敗しました", L"error", MB_OK | MB_ICONERROR);
		return -1;
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
			MessageBox(nullptr, L"CommandQueue の作成に失敗しました", L"error", MB_OK | MB_ICONERROR);
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
