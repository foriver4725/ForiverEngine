#include "../headers/D3D12Helper.h"

#include <string>
#include <functional>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace ForiverEngine
{
	constexpr int FenceValueBeforeGPUEvent = 0;
	constexpr int FenceValueAfterGPUEvent = 1;

	static D3D12_CPU_DESCRIPTOR_HANDLE* Reinterpret(DescriptorHeapHandleAtCPU* value);
	static DescriptorHeapHandleAtCPU* Reinterpret(D3D12_CPU_DESCRIPTOR_HANDLE* value);

	// グラフィックアダプターを順に列挙していき、その Description が最初に Comparer にマッチしたものを返す
	// 見つからなかった場合は nullptr を返す
	static GraphicAdapter FindAvailableGraphicAdapter(const Factory& factory, std::function<bool(const std::wstring&)> descriptionComparer);

	// SwapChain からバッファ数を取得する (失敗したら -1)
	static int GetBufferCountFromSwapChain(const SwapChain& swapChain);

	// DescriptorHeap のハンドルを作成し、index 番目の Descriptor を指し示すように内部ポインタを進めて返す
	// CPU 用
	static DescriptorHeapHandleAtCPU CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
		const Device& device, const DescriptorHeap& descriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, int index);

	// ResourceBarrier() を実行し、GraphicBuffer がどう状態遷移するかをGPUに教える
	static void InvokeResourceBarrierAsTransition(
		const CommandList& commandList, const GraphicBuffer& graphicBuffer,
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	Factory D3D12Helper::CreateFactory()
	{
		IDXGIFactoryLatest* ptr = nullptr;
#ifdef _DEBUG
		if (CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&ptr)) == S_OK)
#else
		if (CreateDXGIFactory1(IID_PPV_ARGS(&ptr)) == S_OK)
#endif
			return Factory(ptr);

		return Factory::Nullptr();
	}

	Device D3D12Helper::CreateDevice(const Factory& factory)
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

	CommandAllocator D3D12Helper::CreateCommandAllocator(const Device& device)
	{
		ID3D12CommandAllocator* ptr = nullptr;
		if (device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&ptr)
		) == S_OK)
		{
			return CommandAllocator(ptr);
		}

		return CommandAllocator::Nullptr();
	}

	CommandList D3D12Helper::CreateCommandList(const Device& device, const CommandAllocator& commandAllocator)
	{
		ID3D12GraphicsCommandList* ptr = nullptr;
		if (device->CreateCommandList(
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

	CommandQueue D3D12Helper::CreateCommandQueue(const Device& device)
	{
		D3D12_COMMAND_QUEUE_DESC desc
		{
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT, // コマンドリストと合わせる
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, // 優先度は通常
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE, // 既定
			.NodeMask = 0, // アダプターが1つなので...
		};

		ID3D12CommandQueue* ptr = nullptr;
		if (device->CreateCommandQueue(&desc, IID_PPV_ARGS(&ptr)) == S_OK)
		{
			return CommandQueue(ptr);
		}

		return CommandQueue::Nullptr();
	}

	SwapChain D3D12Helper::CreateSwapChain(const Factory& factory, const CommandQueue& commandQueue, HWND hwnd, int windowWidth, int windowHeight)
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

		if (factory->CreateSwapChainForHwnd(
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

	DescriptorHeap D3D12Helper::CreateDescriptorHeapRTV(const Device& device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV, // レンダーターゲットビュー用
			.NumDescriptors = 2, // ダブルバッファリングなので、2
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, // シェーダー側には見せない
			.NodeMask = 0, // アダプターが1つなので...
		};

		ID3D12DescriptorHeap* ptr = nullptr;
		if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ptr)) == S_OK)
		{
			return DescriptorHeap(ptr);
		}

		return DescriptorHeap::Nullptr();
	}

	Fence D3D12Helper::CreateFence(const Device& device)
	{
		ID3D12Fence* ptr = nullptr;
		if (device->CreateFence(
			FenceValueBeforeGPUEvent, // 初期化値
			D3D12_FENCE_FLAG_NONE, // 取り合えず NONE
			IID_PPV_ARGS(&ptr)
		) == S_OK)
		{
			return Fence(ptr);
		}

		return Fence::Nullptr();
	}

	GraphicBuffer D3D12Helper::CreateGraphicBuffer1D(const Device& device, int size, bool canMapFromCPU)
	{
		D3D12_HEAP_PROPERTIES heapProperties =
		{
			.Type = canMapFromCPU ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT,
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN, // 難しい設定だが、とりあえずデフォルトで
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN, // 難しい設定だが、とりあえずデフォルトで
			.CreationNodeMask = 0, // アダプターが1つなので...
			.VisibleNodeMask = 0, // アダプターが1つなので...
		};

		D3D12_RESOURCE_DESC resourceDesc =
		{
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Alignment = 0, // 既定値でOK
			.Width = static_cast<UINT64>(size), // 1Dなので...
			.Height = 1, // 1Dなので...
			.DepthOrArraySize = 1, // 1でOK
			.MipLevels = 1, // 1でOK
			.Format = DXGI_FORMAT_UNKNOWN, // 画像ではないので、指定しなくてOK
			.SampleDesc = {.Count = 1, .Quality = 0 }, // アンチエイリアシングはしない. でも Count = 0 だとデータが無いことになってしまうので、1にする
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR, // テクスチャではないので、メモリの連続を示すこの列挙型を使う
			.Flags = D3D12_RESOURCE_FLAG_NONE, // 指定なし
		};

		ID3D12Resource* ptr = nullptr;
		if (device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE, // 指定なし
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPUからは読み取り専用
			nullptr, // 使わない
			IID_PPV_ARGS(&ptr)
		) == S_OK)
		{
			return GraphicBuffer(ptr);
		}

		return GraphicBuffer::Nullptr();
	}

	bool D3D12Helper::CopyDataFromCPUToGPUThroughGraphicBuffer(const GraphicBuffer& graphicBuffer, void* dataBegin, std::size_t dataSize)
	{
		void* bufferVirtualPtr = nullptr;
		if (graphicBuffer->Map(
			0, // リソース配列やミップマップなどではないので、0でOK
			nullptr, // GraphicBuffer の全範囲を対象にしたい
			&bufferVirtualPtr
		) != S_OK)
		{
			return false;
		}

		std::memcpy(bufferVirtualPtr, dataBegin, dataSize);
		graphicBuffer->Unmap(0, nullptr);
		return true;
	}

	bool D3D12Helper::LinkDescriptorHeapRTVToSwapChain(
		const Device& device, const DescriptorHeap& descriptorHeapRTV, const SwapChain& swapChain)
	{
		for (int i = 0; i < GetBufferCountFromSwapChain(swapChain); ++i)
		{
			GraphicBuffer graphicsBuffer = GetBufferByIndex(swapChain, i);
			if (!graphicsBuffer)
				return false;

			DescriptorHeapHandleAtCPU handleRTV = CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
				device, descriptorHeapRTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, i);

			device->CreateRenderTargetView(
				graphicsBuffer.Ptr,
				nullptr, // 今回は nullptr でOK (ミップマップに関係したりする)
				*Reinterpret(&handleRTV) // i 番目の Descriptor (RTV) を指し示すハンドル
			);
		}

		return true;
	}

	bool D3D12Helper::ClearCommandAllocatorAndList(const CommandAllocator& commandAllocator, const CommandList& commandList)
	{
		if (commandAllocator->Reset() != S_OK) return false;
		if (commandList->Reset(commandAllocator.Ptr, nullptr) != S_OK) return false;
		return true;
	}

	int D3D12Helper::GetCurrentBackBufferIndex(const SwapChain& swapChain)
	{
		return swapChain->GetCurrentBackBufferIndex();
	}

	GraphicBuffer D3D12Helper::GetBufferByIndex(const SwapChain& swapChain, int index)
	{
		ID3D12Resource* ptr = nullptr;
		if (swapChain->GetBuffer(index, IID_PPV_ARGS(&ptr)) == S_OK)
		{
			return GraphicBuffer(ptr);
		}

		return GraphicBuffer::Nullptr();
	}

	DescriptorHeapHandleAtCPU D3D12Helper::CreateDescriptorRTVHandleByIndex(
		const Device& device, const DescriptorHeap& descriptorHeapRTV, int index)
	{
		return CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
			device, descriptorHeapRTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, index);
	}

	void D3D12Helper::InvokeResourceBarrierAsTransitionFromPresentToRenderTarget(
		const CommandList& commandList, const GraphicBuffer& graphicBuffer)
	{
		InvokeResourceBarrierAsTransition(
			commandList, graphicBuffer,
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
		);
	}

	void D3D12Helper::InvokeResourceBarrierAsTransitionFromRenderTargetToPresent(
		const CommandList& commandList, const GraphicBuffer& graphicBuffer)
	{
		InvokeResourceBarrierAsTransition(
			commandList, graphicBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
		);
	}

	void D3D12Helper::CommandSetRTAsOutputStage(const CommandList& commandList, const DescriptorHeapHandleAtCPU& handleRTV)
	{
		commandList->OMSetRenderTargets(
			1, // 今のところ、一回の描画における RT は1つのみ
			Reinterpret(const_cast<DescriptorHeapHandleAtCPU*>(&handleRTV)), // Descriptor (RTV) を指し示すハンドル
			true, // Descriptor (RTV) 達はメモリで連続している
			nullptr // Descriptor (DSV) を指し示すハンドル (今のところは nullptr でOK)
		);
	}

	void D3D12Helper::CommandClearRT(const CommandList& commandList, const DescriptorHeapHandleAtCPU& handleRTV, float clearColor4[])
	{
		// 第3,4引数は、クリアする範囲を指定する
		// 今回は画面全体をクリアするので、指定する必要はない
		commandList->ClearRenderTargetView(*Reinterpret(const_cast<DescriptorHeapHandleAtCPU*>(&handleRTV)), clearColor4, 0, nullptr);
	}

	void D3D12Helper::CommandClose(const CommandList& commandList)
	{
		commandList->Close();
	}

	void D3D12Helper::ExecuteCommands(const CommandQueue& commandQueue, const CommandList& commandList)
	{
		// コマンドリストは1つのみ
		ID3D12CommandList* commandListPointers[] = { commandList.Ptr };
		commandQueue->ExecuteCommandLists(1, commandListPointers);
	}

	bool D3D12Helper::WaitForGPUEventCompletion(const Fence& fence, const CommandQueue& commandQueue)
	{
		// CPU側で、GPU側の処理終了時に期待されるフェンス値を渡す
		// GetCompletedValue() は GPU からのフェンス値を返すので、この等価比較で同期をとる
		if (commandQueue->Signal(fence.Ptr, FenceValueAfterGPUEvent) != S_OK)
			return false;

		if (fence->GetCompletedValue() != FenceValueAfterGPUEvent)
		{
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			if (event)
			{
				fence->SetEventOnCompletion(FenceValueAfterGPUEvent, event);
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}
			else
				return false;
		}

		return true;
	}

	bool D3D12Helper::Present(const SwapChain& swapChain)
	{
		// 第1引数は、フリップまでの待機フレーム数 (= 待つべき垂直同期の数)
		// バックバッファーが2枚のみなので、今は1で良い (フルスクリーン or ウィンドウ によっても変わる)
		// 第2引数は、特殊用途での指定が多い (今は必要ないので 0)
		return swapChain->Present(1, 0) == S_OK;
	}

#ifdef _DEBUG
	bool D3D12Helper::EnableDebugLayer()
	{
		ID3D12Debug* debugLayer = nullptr;
		if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)) == S_OK)
		{
			debugLayer->EnableDebugLayer();
			debugLayer->Release();

			return true;
		}

		return false;
	}
#endif
	static D3D12_CPU_DESCRIPTOR_HANDLE* Reinterpret(DescriptorHeapHandleAtCPU* value)
	{
		return reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(value);
	}

	static DescriptorHeapHandleAtCPU* Reinterpret(D3D12_CPU_DESCRIPTOR_HANDLE* value)
	{
		return reinterpret_cast<DescriptorHeapHandleAtCPU*>(value);
	}


	GraphicAdapter FindAvailableGraphicAdapter(const Factory& factory, std::function<bool(const std::wstring&)> descriptionComparer)
	{
		IDXGIAdapter* ptr = nullptr;
		for (int i = 0; factory->EnumAdapters(i, &ptr) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC desc;
			ptr->GetDesc(&desc);

			if (descriptionComparer(desc.Description))
				return GraphicAdapter(ptr);
		}

		return GraphicAdapter::Nullptr();
	}

	int GetBufferCountFromSwapChain(const SwapChain& swapChain)
	{
		DXGI_SWAP_CHAIN_DESC desc = {};
		if (swapChain->GetDesc(&desc) == S_OK)
		{
			return desc.BufferCount;
		}

		return -1;
	}

	DescriptorHeapHandleAtCPU CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
		const Device& device, const DescriptorHeap& descriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, int index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += index * device->GetDescriptorHandleIncrementSize(descriptorHeapType);
		return *Reinterpret(&handle);
	}

	void InvokeResourceBarrierAsTransition(
		const CommandList& commandList, const GraphicBuffer& graphicBuffer,
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
	{
		D3D12_RESOURCE_BARRIER desc =
		{
			.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, // 状態遷移
			.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, // 特別なことはしないので、指定しない
			.Transition = // union なので、これじゃないメンバは触らないこと!
			{
				.pResource = graphicBuffer.Ptr, // GraphicBuffer のアドレスをそのまま渡せば良い
				.Subresource = 0, // 今回はバックバッファーが1つしかないので、まとめて指定するなどは必要ない
				.StateBefore = beforeState,
				.StateAfter = afterState,
			},
		};

		// 配列で渡せるが、今回はバリアは1つだけ
		commandList->ResourceBarrier(1, &desc);
	}
}
