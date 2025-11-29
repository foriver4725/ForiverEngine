#include "../headers/D3D12ObjectFactory.h"

#include <string>
#include <functional>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace ForiverEngine
{
	// グラフィックアダプターを順に列挙していき、その Description が最初に Comparer にマッチしたものを返す
	// 見つからなかった場合は nullptr を返す
	static GraphicAdapter FindAvailableGraphicAdapter(const Factory& factory, std::function<bool(const std::wstring&)> descriptionComparer);

	Factory D3D12ObjectFactory::CreateFactory()
	{
		IDXGIFactoryLatest* ptr = nullptr;
		if (CreateDXGIFactory1(IID_PPV_ARGS(&ptr)) == S_OK)
			return Factory(ptr);

		return Factory::Nullptr();
	}

	Device D3D12ObjectFactory::CreateDevice(const Factory& factory)
	{
		// 上から順に、対応している機能レベルを探して行く
		constexpr D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		// 利用可能なアダプターを取得 (デバイスを作成時に渡す)
		// 判定は緩いが、nullptr でもOKなので、一旦これで行く
		GraphicAdapter adapter = FindAvailableGraphicAdapter(factory, [](const auto& desc) -> bool
			{
				return desc.find(L"NVIDIA") != std::string::npos;
			});

		for (auto featureLevel : featureLevels)
		{
			ID3D12DeviceLatest* ptr = nullptr;
			if (D3D12CreateDevice(adapter.Ptr, featureLevel, IID_PPV_ARGS(&ptr)) == S_OK)
			{
				return Device(ptr);
			}
		}

		return Device::Nullptr();
	}

	CommandAllocator D3D12ObjectFactory::CreateCommandAllocator(const Device& device)
	{
		ID3D12CommandAllocator* ptr = nullptr;
		if (device.Ptr->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&ptr)
		) == S_OK)
		{
			return CommandAllocator(ptr);
		}

		return CommandAllocator::Nullptr();
	}

	CommandList D3D12ObjectFactory::CreateCommandList(const Device& device, const CommandAllocator& commandAllocator)
	{
		ID3D12GraphicsCommandList* ptr = nullptr;
		if (device.Ptr->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			commandAllocator.Ptr,
			nullptr,
			IID_PPV_ARGS(&ptr)
		) == S_OK)
		{
			return CommandList(ptr);
		}

		return CommandList::Nullptr();
	}

	CommandQueue D3D12ObjectFactory::CreateCommandQueue(const Device& device)
	{
		D3D12_COMMAND_QUEUE_DESC desc
		{
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストと合わせる
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, // 優先度は通常
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE, // 既定
			.NodeMask = 0, // アダプターが1つなので...
		};

		ID3D12CommandQueue* ptr = nullptr;
		if (device.Ptr->CreateCommandQueue(&desc, IID_PPV_ARGS(&ptr)) == S_OK)
		{
			return CommandQueue(ptr);
		}

		return CommandQueue::Nullptr();
	}

	SwapChain D3D12ObjectFactory::CreateSwapChain(const Factory& factory, const CommandQueue& commandQueue, HWND hwnd, int windowWidth, int windowHeight)
	{
		DXGI_SWAP_CHAIN_DESC1 desc
		{
			.Width = static_cast<UINT>(windowWidth),
			.Height = static_cast<UINT>(windowHeight),
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = false,
			.SampleDesc = {.Count = 1, .Quality = 0 }, // マルチサンプルの指定
			.BufferUsage = DXGI_USAGE_BACK_BUFFER, // これでOK
			.BufferCount = 2, // ダブルバッファリングなので、2
			.Scaling = DXGI_SCALING_STRETCH, // 画面に合わせて伸縮
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD, // フリップ後、速やかに破棄
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED, // 特に指定なし
			.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH, // ウィンドウ<->フルスクリーンの切り替えを許可
		};

		IDXGISwapChainLatest* ptr = nullptr;

		// 本来は QueryInterface などを使うが、ここでは簡単なのでこれで行く
		IDXGISwapChain1** ptrAs1 = reinterpret_cast<IDXGISwapChain1**>(&ptr);
		if (!ptrAs1)
		{
			return SwapChain::Nullptr();
		}

		if (factory.Ptr->CreateSwapChainForHwnd(
			commandQueue.Ptr, // 注意 : コマンドキュー
			hwnd,
			&desc,
			nullptr, // ひとまずnullptrでOK
			nullptr, // ひとまずnullptrでOK
			ptrAs1
		) == S_OK)
		{
			return SwapChain(ptr);
		}

		return SwapChain::Nullptr();
	}

	DescriptorHeap D3D12ObjectFactory::CreateDescriptorHeap(const Device& device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, // レンダーターゲットビュー用
			.NumDescriptors = 2, // ダブルバッファリングなので、2
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, // シェーダー側には見せない
			.NodeMask = 0, // アダプターが1つなので...
		};

		ID3D12DescriptorHeap* ptr = nullptr;
		if (device.Ptr->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ptr)) == S_OK)
		{
			return DescriptorHeap(ptr);
		}

		return DescriptorHeap::Nullptr();
	}

	GraphicAdapter FindAvailableGraphicAdapter(const Factory& factory, std::function<bool(const std::wstring&)> descriptionComparer)
	{
		IDXGIAdapter* ptr = nullptr;
		for (int i = 0; factory.Ptr->EnumAdapters(i, &ptr) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC desc;
			ptr->GetDesc(&desc);

			if (descriptionComparer(desc.Description))
				return GraphicAdapter(ptr);
		}

		return GraphicAdapter::Nullptr();
	}
}
