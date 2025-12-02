#include "../headers/D3D12Helper.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace ForiverEngine
{
	constexpr int FenceValueBeforeGPUEvent = 0;
	constexpr int FenceValueAfterGPUEvent = 1;

	static D3D12_CPU_DESCRIPTOR_HANDLE* Reinterpret(DescriptorHeapHandleAtCPU* value);
	static DescriptorHeapHandleAtCPU* Reinterpret(D3D12_CPU_DESCRIPTOR_HANDLE* value);

	// エラーの Blob からエラーメッセージを取得する
	static std::wstring FetchErrorMessageFromErrorBlob(const Blob& blob);

	// グラフィックアダプターを順に列挙していき、その Description が最初に Comparer にマッチしたものを返す
	// 見つからなかった場合は nullptr を返す
	static GraphicAdapter FindAvailableGraphicAdapter(const Factory& factory, std::function<bool(const std::wstring&)> descriptionComparer);

	// SwapChain からバッファ数を取得する (失敗したら -1)
	static int GetBufferCountFromSwapChain(const SwapChain& swapChain);

	// DescriptorHeap のハンドルを作成し、index 番目の Descriptor を指し示すように内部ポインタを進めて返す
	// CPU 用
	static DescriptorHeapHandleAtCPU CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
		const Device& device, const DescriptorHeap& descriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, int index);

	// ResourceBarrier() を実行し、GraphicsBuffer がどう状態遷移するかをGPUに教える
	static void InvokeResourceBarrierAsTransition(
		const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer,
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

		return Factory();
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

		return Device();
	}

	// TODO: 作成途中
	RootSignature D3D12Helper::CreateRootSignature(const Device& device, std::wstring& outErrorMessage)
	{
		D3D12_ROOT_SIGNATURE_DESC desc
		{
			.NumParameters = 0, // 指定なし
			.pParameters = nullptr, // 指定なし
			.NumStaticSamplers = 0, // 指定なし
			.pStaticSamplers = nullptr, // 指定なし
			.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, // 「頂点情報(入力アセンブラ) がある」
		};

		ID3DBlob* blob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		if (D3D12SerializeRootSignature(
			&desc,
			D3D_ROOT_SIGNATURE_VERSION_1_0, // バージョン間で仕様が違ったりするので、今回は 1.0 を指定
			&blob,
			&errorBlob
		) != S_OK)
		{
			outErrorMessage = FetchErrorMessageFromErrorBlob(Blob(errorBlob));
			blob->Release();
			errorBlob->Release();
			return RootSignature();
		}

		ID3D12RootSignature* ptr = nullptr;
		if (device->CreateRootSignature(
			0, // ノードマスク (アダプターが1つなので...)
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			IID_PPV_ARGS(&ptr)
		) != S_OK)
		{
			outErrorMessage = L"RootSignature の作成に失敗しました";
			blob->Release();
			errorBlob->Release();
			return RootSignature();
		}

		outErrorMessage = L"";
		blob->Release();
		errorBlob->Release();
		return RootSignature(ptr);
	}

	PipelineState D3D12Helper::CreateGraphicsPipelineState(
		const Device& device, const Blob& vs, const Blob& ps,
		const std::vector<VertexLayout>& vertexLayouts, int eFillMode, int eCullMode)
	{
		std::vector< D3D12_INPUT_ELEMENT_DESC> vertexLayoutsReal = std::vector< D3D12_INPUT_ELEMENT_DESC>(vertexLayouts.size(), {});
		for (int i = 0; i < static_cast<int>(vertexLayouts.size()); ++i)
		{
			vertexLayoutsReal[i] =
			{
				.SemanticName = vertexLayouts[i].SemanticName,
				.SemanticIndex = 0, // 同じセマンティクス名が複数ある場合のインデックス (0でOK)
				.Format = static_cast<DXGI_FORMAT>(vertexLayouts[i].Format),
				.InputSlot = 0, // インターリーブ なので、0番スロットだけ使えばOK
				.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, // 頂点バッファのメンバは連続しているので、それらのアドレスは自動で計算してもらう
				.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // 1頂点ごとにこのレイアウトが入っている
				.InstanceDataStepRate = 0, // インスタンシングではないので、データは使いまわさない
			};
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc =
		{
			.pRootSignature = nullptr, // 一旦 nullptr にする!!!
			.VS = {.pShaderBytecode = vs->GetBufferPointer(), .BytecodeLength = vs->GetBufferSize() },
			.PS = {.pShaderBytecode = ps->GetBufferPointer(), .BytecodeLength = ps->GetBufferSize() },
			.DS = {}, // ドメインシェーダー (使わない)
			.HS = {}, // ハルシェーダー (使わない)
			.GS = {}, // ジオメトリシェーダー (使わない)
			.StreamOutput = {}, // ストリーミング出力バッファー設定 (不要)
			.BlendState =
			{
				.AlphaToCoverageEnable = false, // aテスト無効
				.IndependentBlendEnable = false, // MRT ではないので... (RenderTarget[0] の設定が全てに適用される)
				.RenderTarget =
				{ // このコメントを消すと、コードの自動成形が変になる...
					// [0]
					{
						.BlendEnable = false, // 今回はブレンドしない
						.LogicOpEnable = false, // 今回はブレンドしない
						.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL, // RGBA 全てブレンドする
					},

					// [1] ~ [7]
				},
			},
			.SampleMask = D3D12_DEFAULT_SAMPLE_MASK, // デフォルト
			.RasterizerState =
			{
				.FillMode = static_cast<D3D12_FILL_MODE>(eFillMode), // 塗りつぶし or ワイヤーフレーム
				.CullMode = static_cast<D3D12_CULL_MODE>(eCullMode), // カリング (None, Front, Back)
				.DepthClipEnable = true, // 深度クリッピング有効
				.MultisampleEnable = false, // まだアンチエイリアスは使わないので...
			},
			.DepthStencilState = {}, // 深度ステンシルバッファの設定 (今回は指定しない)
			.InputLayout = // 頂点レイアウト
			{
				.pInputElementDescs = vertexLayoutsReal.data(),
				.NumElements = static_cast<UINT>(vertexLayoutsReal.size()),
			},
			.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED, // トライアングルリスト (トライアングルストリップ ではない) なので、無効化
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, // 三角形
			.NumRenderTargets = 1, // MRT ではないので、1 でOK
			.RTVFormats =
			{
				// [0]
				DXGI_FORMAT_R8G8B8A8_UNORM, // RT のフォーマット. [0, 1] に正規化された RGBA. RT は1つなので、[0] のみ指定

				// [1] ~ [7]
			},
			.DSVFormat = {}, // 深度ステンシルバッファのフォーマット (今回は指定しない)
			.SampleDesc = {.Count = 1, .Quality = 0 }, // アンチエイリアシングはしない (1ピクセルあたり1回サンプリング)
			.NodeMask = 0, // アダプターが1つなので...
			.CachedPSO = {}, // 高速化出来るけど、今は使わない
			.Flags = D3D12_PIPELINE_STATE_FLAG_NONE, // 指定なし
		};

		ID3D12PipelineState* ptr = nullptr;
		if (device->CreateGraphicsPipelineState(
			&desc,
			IID_PPV_ARGS(&ptr)
		) == S_OK)
		{
			return PipelineState(ptr);
		}

		return PipelineState();
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

		return CommandAllocator();
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

		return CommandList();
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

		return CommandQueue();
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
			return SwapChain();
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

		return SwapChain();
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

		return DescriptorHeap();
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

		return Fence();
	}

	GraphicsBuffer D3D12Helper::CreateGraphicsBuffer1D(const Device& device, int size, bool canMapFromCPU)
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
			return GraphicsBuffer(ptr);
		}

		return GraphicsBuffer();
	}

	bool D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer(const GraphicsBuffer& GraphicsBuffer, void* dataBegin, std::size_t dataSize)
	{
		void* bufferVirtualPtr = nullptr;
		if (GraphicsBuffer->Map(
			0, // リソース配列やミップマップなどではないので、0でOK
			nullptr, // GraphicsBuffer の全範囲を対象にしたい
			&bufferVirtualPtr
		) != S_OK)
		{
			return false;
		}

		std::memcpy(bufferVirtualPtr, dataBegin, dataSize);
		GraphicsBuffer->Unmap(0, nullptr);
		return true;
	}

	bool D3D12Helper::LinkDescriptorHeapRTVToSwapChain(
		const Device& device, const DescriptorHeap& descriptorHeapRTV, const SwapChain& swapChain)
	{
		for (int i = 0; i < GetBufferCountFromSwapChain(swapChain); ++i)
		{
			GraphicsBuffer graphicsBuffer = GetBufferByIndex(swapChain, i);
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

	GraphicsBuffer D3D12Helper::GetBufferByIndex(const SwapChain& swapChain, int index)
	{
		ID3D12Resource* ptr = nullptr;
		if (swapChain->GetBuffer(index, IID_PPV_ARGS(&ptr)) == S_OK)
		{
			return GraphicsBuffer(ptr);
		}

		return GraphicsBuffer();
	}

	DescriptorHeapHandleAtCPU D3D12Helper::CreateDescriptorRTVHandleByIndex(
		const Device& device, const DescriptorHeap& descriptorHeapRTV, int index)
	{
		return CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
			device, descriptorHeapRTV, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, index);
	}

	void D3D12Helper::InvokeResourceBarrierAsTransitionFromPresentToRenderTarget(
		const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer)
	{
		InvokeResourceBarrierAsTransition(
			commandList, GraphicsBuffer,
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
		);
	}

	void D3D12Helper::InvokeResourceBarrierAsTransitionFromRenderTargetToPresent(
		const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer)
	{
		InvokeResourceBarrierAsTransition(
			commandList, GraphicsBuffer,
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

	void D3D12Helper::CommandClearRT(
		const CommandList& commandList, const DescriptorHeapHandleAtCPU& handleRTV, const std::array<float, 4>& clearColor)
	{
		// 第3,4引数は、クリアする範囲を指定する
		// 今回は画面全体をクリアするので、指定する必要はない
		commandList->ClearRenderTargetView(*Reinterpret(const_cast<DescriptorHeapHandleAtCPU*>(&handleRTV)), clearColor.data(), 0, nullptr);
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

	Blob D3D12Helper::CompileShaderFile(
		const std::wstring& path, const std::string& entryFunc, const std::string& shaderTarget, std::wstring& outErrorMessage)
	{
		ID3DBlob* blob = nullptr;
		ID3DBlob* errorBlob = nullptr;

#ifdef _DEBUG
		constexpr UINT ShaderCompileOption = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		constexpr UINT ShaderCompileOption = 0;
#endif

		HRESULT result = D3DCompileFromFile(
			path.c_str(),
			nullptr, // #define みたいなのを配列で指定
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // こうすることで、シェーダー内に #include がある場合、カレントディレクトリから探すようになる
			entryFunc.c_str(),
			shaderTarget.c_str(),
			ShaderCompileOption,
			0, // シェーダーファイルの場合、0が推奨されている
			&blob,
			&errorBlob
		);

		if (result == S_OK)
		{
			outErrorMessage = L"";
			return Blob(blob);
		}

		if (FAILED(result) && result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			outErrorMessage = L"Shader file not found.";
			return Blob();
		}

		outErrorMessage = FetchErrorMessageFromErrorBlob(Blob(errorBlob));
		return Blob();
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

	D3D12_CPU_DESCRIPTOR_HANDLE* Reinterpret(DescriptorHeapHandleAtCPU* value)
	{
		return reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(value);
	}

	DescriptorHeapHandleAtCPU* Reinterpret(D3D12_CPU_DESCRIPTOR_HANDLE* value)
	{
		return reinterpret_cast<DescriptorHeapHandleAtCPU*>(value);
	}

	std::wstring FetchErrorMessageFromErrorBlob(const Blob& blob)
	{
		std::wstring message;

		message.resize(blob->GetBufferSize());
		std::copy_n(static_cast<char*>(blob->GetBufferPointer()), blob->GetBufferSize(), message.begin());
		message += L"\n";

		return message;
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

		return GraphicAdapter();
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
		const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer,
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
	{
		D3D12_RESOURCE_BARRIER desc =
		{
			.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, // 状態遷移
			.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, // 特別なことはしないので、指定しない
			.Transition = // union なので、これじゃないメンバは触らないこと!
			{
				.pResource = GraphicsBuffer.Ptr, // GraphicsBuffer のアドレスをそのまま渡せば良い
				.Subresource = 0, // 今回はバックバッファーが1つしかないので、まとめて指定するなどは必要ない
				.StateBefore = beforeState,
				.StateAfter = afterState,
			},
		};

		// 配列で渡せるが、今回はバリアは1つだけ
		commandList->ResourceBarrier(1, &desc);
	}
}
