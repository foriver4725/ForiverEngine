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

	static D3D12_DESCRIPTOR_RANGE Construct(const RootParameter::DescriptorRange& descriptorRange);
	static D3D12_STATIC_SAMPLER_DESC Construct(const SamplerConfig& samplerConfig);
	static D3D12_INPUT_ELEMENT_DESC Construct(const VertexLayout& vertexLayout);
	static std::pair<D3D12_VIEWPORT, D3D12_RECT> Construct(const ViewportScissorRect& viewportScissorRect);

	static D3D12_CPU_DESCRIPTOR_HANDLE* Reinterpret(const DescriptorHeapHandleAtCPU* value);
	static DescriptorHeapHandleAtCPU* Reinterpret(const D3D12_CPU_DESCRIPTOR_HANDLE* value);
	static D3D12_GPU_DESCRIPTOR_HANDLE* Reinterpret(const DescriptorHeapHandleAtGPU* value);
	static DescriptorHeapHandleAtGPU* Reinterpret(const D3D12_GPU_DESCRIPTOR_HANDLE* value);
	static D3D12_VERTEX_BUFFER_VIEW* Reinterpret(const VertexBufferView* value);
	static VertexBufferView* Reinterpret(const D3D12_VERTEX_BUFFER_VIEW* value);
	static D3D12_INDEX_BUFFER_VIEW* Reinterpret(const IndexBufferView* value);
	static IndexBufferView* Reinterpret(const D3D12_INDEX_BUFFER_VIEW* value);

	// エラーの Blob からエラーメッセージを取得する
	static std::wstring FetchErrorMessageFromErrorBlob(const Blob& blob);

	// グラフィックアダプターを順に列挙していき、その Description が最初に Comparer にマッチしたものを返す
	// 見つからなかった場合は nullptr を返す
	static GraphicAdapter FindAvailableGraphicAdapter(const Factory& factory, std::function<bool(const std::wstring&)> descriptionComparer);

	// SwapChain からバッファ数を取得する (失敗したら -1)
	static int GetBufferCountFromSwapChain(const SwapChain& swapChain);

	Factory D3D12Helper::CreateFactory()
	{
		IDXGIFactoryLatest* ptr = nullptr;
#ifdef _DEBUG
		if (SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&ptr))))
#else
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&ptr))))
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
			if (SUCCEEDED(D3D12CreateDevice(adapter.Ptr, featureLevel, IID_PPV_ARGS(&ptr))))
			{
				return Device(ptr);
			}
		}

		return Device();
	}

	RootSignature D3D12Helper::CreateRootSignature(
		const Device& device, const RootParameter& rootParameter, const SamplerConfig& samplerConfig, std::wstring& outErrorMessage)
	{
		// 同じ種類の Descriptor が複数連続している場合、まとめて指定できるようにするためのもの
		std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRangesReal = std::vector<D3D12_DESCRIPTOR_RANGE>(rootParameter.descriptorRanges.size(), {});
		for (int i = 0; i < static_cast<int>(rootParameter.descriptorRanges.size()); ++i)
			descriptorRangesReal[i] = Construct(rootParameter.descriptorRanges[i]);

		// Descriptor の総数を算出する
		int totalDescriptorAmount = 0;
		for (const auto& range : rootParameter.descriptorRanges)
			totalDescriptorAmount += range.amount;

		const D3D12_ROOT_PARAMETER rootParameterReal =
		{
			.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			.DescriptorTable =
			{
				.NumDescriptorRanges = static_cast<UINT>(totalDescriptorAmount),
				.pDescriptorRanges = descriptorRangesReal.data()
			},
			.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(rootParameter.shaderVisibility)
		};

		const D3D12_STATIC_SAMPLER_DESC samplerDescReal = Construct(samplerConfig);

		const D3D12_ROOT_SIGNATURE_DESC desc
		{
			.NumParameters = 1,
			.pParameters = &rootParameterReal,
			.NumStaticSamplers = 1,
			.pStaticSamplers = &samplerDescReal,
			.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, // 「頂点情報(入力アセンブラ) がある」
		};

		ID3DBlob* blob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		if (FAILED(D3D12SerializeRootSignature(
			&desc,
			D3D_ROOT_SIGNATURE_VERSION_1_0, // バージョン間で仕様が違ったりするので、今回は 1.0 を指定
			&blob,
			&errorBlob
		)))
		{
			outErrorMessage = FetchErrorMessageFromErrorBlob(Blob(errorBlob));
			if (blob) blob->Release();
			if (errorBlob) errorBlob->Release();
			return RootSignature();
		}

		ID3D12RootSignature* ptr = nullptr;
		if (FAILED(device->CreateRootSignature(
			0, // ノードマスク (アダプターが1つなので...)
			blob->GetBufferPointer(),
			blob->GetBufferSize(),
			IID_PPV_ARGS(&ptr)
		)))
		{
			outErrorMessage = L"RootSignature の作成に失敗しました";
			if (blob) blob->Release();
			if (errorBlob) errorBlob->Release();
			return RootSignature();
		}

		outErrorMessage = L"";
		if (blob) blob->Release();
		if (errorBlob) errorBlob->Release();
		return RootSignature(ptr);
	}

	PipelineState D3D12Helper::CreateGraphicsPipelineState(
		const Device& device, const RootSignature& rootSignature, const Blob& vs, const Blob& ps,
		const std::vector<VertexLayout>& vertexLayouts, FillMode fillMode, CullMode cullMode)
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> vertexLayoutsReal = std::vector<D3D12_INPUT_ELEMENT_DESC>(vertexLayouts.size(), {});
		for (int i = 0; i < static_cast<int>(vertexLayouts.size()); ++i)
			vertexLayoutsReal[i] = Construct(vertexLayouts[i]);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc =
		{
			.pRootSignature = rootSignature.Ptr,
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
				.FillMode = static_cast<D3D12_FILL_MODE>(fillMode), // 塗りつぶし or ワイヤーフレーム
				.CullMode = static_cast<D3D12_CULL_MODE>(cullMode), // カリング (None, Front, Back)
				.DepthClipEnable = true, // 深度クリッピング有効
				.MultisampleEnable = false, // まだアンチエイリアスは使わないので...
			},
			.DepthStencilState =
			{
				.DepthEnable = true,
				.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL, // 深度値を書き込む
				.DepthFunc = D3D12_COMPARISON_FUNC_LESS, // 深度値がより小さいなら、描画して良い (手前にあるので)
				.StencilEnable = false, // ステンシルは使わない!
			},
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
				static_cast<DXGI_FORMAT>(Format::RGBA_U8_01), // バックバッファがあるけど、使われる RT は1つなので、[0] のみ指定すれば良い

				// [1] ~ [7]
			},
			.DSVFormat = static_cast<DXGI_FORMAT>(Format::D_F32),
			.SampleDesc = {.Count = 1, .Quality = 0 }, // アンチエイリアシングはしない (1ピクセルあたり1回サンプリング)
			.NodeMask = 0, // アダプターが1つなので...
			.CachedPSO = {}, // 高速化出来るけど、今は使わない
			.Flags = D3D12_PIPELINE_STATE_FLAG_NONE, // 指定なし
		};

		ID3D12PipelineState* ptr = nullptr;
		if (SUCCEEDED(device->CreateGraphicsPipelineState(
			&desc,
			IID_PPV_ARGS(&ptr)
		)))
		{
			return PipelineState(ptr);
		}

		return PipelineState();
	}

	CommandAllocator D3D12Helper::CreateCommandAllocator(const Device& device)
	{
		ID3D12CommandAllocator* ptr = nullptr;
		if (SUCCEEDED(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&ptr)
		)))
		{
			return CommandAllocator(ptr);
		}

		return CommandAllocator();
	}

	CommandList D3D12Helper::CreateCommandList(const Device& device, const CommandAllocator& commandAllocator)
	{
		ID3D12GraphicsCommandList* ptr = nullptr;
		if (SUCCEEDED(device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			commandAllocator.Ptr,
			nullptr,
			IID_PPV_ARGS(&ptr)
		)))
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
		if (SUCCEEDED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&ptr))))
		{
			return CommandQueue(ptr);
		}

		return CommandQueue();
	}

	SwapChain D3D12Helper::CreateSwapChain(const Factory& factory, const CommandQueue& commandQueue, HWND hwnd, int windowWidth, int windowHeight)
	{
		const DXGI_SWAP_CHAIN_DESC1 desc
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

		if (SUCCEEDED(factory->CreateSwapChainForHwnd(
			commandQueue.Ptr, // 注意 : コマンドキュー
			hwnd,
			&desc,
			nullptr, // ひとまずnullptrでOK
			nullptr, // ひとまずnullptrでOK
			ptrAs1
		)))
		{
			return SwapChain(ptr);
		}

		return SwapChain();
	}

	DescriptorHeap D3D12Helper::CreateDescriptorHeap(const Device& device, DescriptorHeapType type, int descriptorAmount, bool visibleToShader)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc
		{
			.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type),
			.NumDescriptors = static_cast<UINT>(descriptorAmount),
			.Flags = visibleToShader ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0, // アダプターが1つなので...
		};

		ID3D12DescriptorHeap* ptr = nullptr;
		if (SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ptr))))
		{
			return DescriptorHeap(ptr);
		}

		return DescriptorHeap();
	}

	Fence D3D12Helper::CreateFence(const Device& device)
	{
		ID3D12Fence* ptr = nullptr;
		if (SUCCEEDED(device->CreateFence(
			FenceValueBeforeGPUEvent, // 初期化値
			D3D12_FENCE_FLAG_NONE, // 取り合えず NONE
			IID_PPV_ARGS(&ptr)
		)))
		{
			return Fence(ptr);
		}

		return Fence();
	}

	GraphicsBuffer D3D12Helper::CreateGraphicsBuffer1D(const Device& device, int size, bool canMapFromCPU)
	{
		const D3D12_HEAP_PROPERTIES heapProperties =
		{
			.Type = canMapFromCPU ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT,
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN, // 難しい設定だが、とりあえずデフォルトで
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN, // 難しい設定だが、とりあえずデフォルトで
			.CreationNodeMask = 0, // アダプターが1つなので...
			.VisibleNodeMask = 0, // アダプターが1つなので...
		};

		const D3D12_RESOURCE_DESC resourceDesc =
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
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE, // 指定なし
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPUからは読み取り専用
			nullptr, // 使わない
			IID_PPV_ARGS(&ptr)
		)))
		{
			return GraphicsBuffer(ptr);
		}

		return GraphicsBuffer();
	}

	GraphicsBuffer D3D12Helper::CreateGraphicsBufferTexture2D(const Device& device, const Texture& texture)
	{
		if (texture.textureType != GraphicsBufferType::Texture2D)
		{
			return GraphicsBuffer();
		}

		const D3D12_HEAP_PROPERTIES heapProperties =
		{
			.Type = D3D12_HEAP_TYPE_DEFAULT, // テクスチャ用
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN, // 規定値
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN, // 規定値
			.CreationNodeMask = 0, // アダプターが1つなので...
			.VisibleNodeMask = 0, // アダプターが1つなので...
		};

		const D3D12_RESOURCE_DESC resourceDesc =
		{
			.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(GraphicsBufferType::Texture2D),
			.Alignment = 0, // 既定値でOK
			.Width = static_cast<UINT64>(texture.width),
			.Height = static_cast<UINT>(texture.height),
			.DepthOrArraySize = static_cast<UINT16>(texture.sliceCount), // 配列のサイズ = スライス数
			.MipLevels = static_cast<UINT16>(texture.mipLevels), // ミップマップ数
			.Format = static_cast<DXGI_FORMAT>(texture.format),
			.SampleDesc = {.Count = 1, .Quality = 0 }, // 通常テクスチャなのでアンチエイリアシングはしない (クオリティは最低)
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, // 決定しない
			.Flags = D3D12_RESOURCE_FLAG_NONE, // 指定なし
		};

		ID3D12Resource* ptr = nullptr;
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE, // 指定なし
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, // コピー先として使う
			nullptr, // 使わない
			IID_PPV_ARGS(&ptr)
		)))
		{
			return GraphicsBuffer(ptr);
		}

		return GraphicsBuffer();
	}

	GraphicsBuffer D3D12Helper::CreateGraphicsBufferTexture2DAsDepthBuffer(const Device& device, int width, int height, float clearValue)
	{
		const D3D12_HEAP_PROPERTIES heapProperties =
		{
			.Type = D3D12_HEAP_TYPE_DEFAULT, // テクスチャ用
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN, // 規定値
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN, // 規定値
			.CreationNodeMask = 0, // アダプターが1つなので...
			.VisibleNodeMask = 0, // アダプターが1つなので...
		};

		const D3D12_RESOURCE_DESC resourceDesc =
		{
			.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(GraphicsBufferType::Texture2D),
			.Alignment = 0, // 既定値
			.Width = static_cast<UINT64>(width),
			.Height = static_cast<UINT>(height),
			.DepthOrArraySize = 1, // テクスチャ配列でも3Dテクスチャでもない
			.MipLevels = 0, // 規定値
			.Format = static_cast<DXGI_FORMAT>(Format::D_F32),
			.SampleDesc = {.Count = 1, .Quality = 0 }, // アンチエイリアシングなし
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN, // 決定しない
			.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, // 深度ステンシルバッファとして使う
		};

		const D3D12_CLEAR_VALUE clearValueDesc =
		{
			.Format = static_cast<DXGI_FORMAT>(Format::D_F32),
			.DepthStencil =
			{
				.Depth = static_cast<FLOAT>(clearValue),
				.Stencil = 0, // 規定値
			}
		};

		ID3D12Resource* ptr = nullptr;
		if (SUCCEEDED(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE, // 指定なし
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値書き込み用
			&clearValueDesc,
			IID_PPV_ARGS(&ptr)
		)))
		{
			return GraphicsBuffer(ptr);
		}

		return GraphicsBuffer();
	}

	bool D3D12Helper::CreateRenderTargetViews(
		const Device& device, const DescriptorHeap& descriptorHeapRTV, const SwapChain& swapChain, bool sRGB)
	{
		// NOTE : sRGB を true にした場合、バックバッファービューとレンダーターゲットフォーマットに食い違いが生じるため、
		//        デバッグレイヤーをオンにしている場合にエラーが表示される

		const Format format = sRGB ? Format::RGBA_U8_01_SRGB : Format::RGBA_U8_01;

		const D3D12_RENDER_TARGET_VIEW_DESC desc =
		{
			.Format = static_cast<DXGI_FORMAT>(format),
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
		};

		for (int i = 0; i < GetBufferCountFromSwapChain(swapChain); ++i)
		{
			const GraphicsBuffer graphicsBuffer = GetBufferByIndex(swapChain, i);
			if (!graphicsBuffer)
				return false;

			const DescriptorHeapHandleAtCPU handleRTV = CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
				device, descriptorHeapRTV, DescriptorHeapType::RTV, i);

			device->CreateRenderTargetView(
				graphicsBuffer.Ptr,
				&desc,
				*Reinterpret(&handleRTV) // i 番目の Descriptor (RTV) を指し示すハンドル
			);
		}

		return true;
	}

	void D3D12Helper::CreateDepthStencilView(
		const Device& device, const DescriptorHeap& descriptorHeapDSV, const GraphicsBuffer& depthBuffer)
	{
		const D3D12_DEPTH_STENCIL_VIEW_DESC desc =
		{
			.Format = static_cast<DXGI_FORMAT>(Format::D_F32),
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			.Flags = D3D12_DSV_FLAG_NONE, // 指定なし
		};

		// DescriptorHeap の 0 番目に DSV を作成する
		const DescriptorHeapHandleAtCPU handleRTV = CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
			device, descriptorHeapDSV, DescriptorHeapType::DSV, 0);

		device->CreateDepthStencilView(
			depthBuffer.Ptr,
			&desc,
			*Reinterpret(&handleRTV)
		);
	}

	VertexBufferView D3D12Helper::CreateVertexBufferView(const GraphicsBuffer& vertexBuffer, int verticesSize, int vertexSize)
	{
		D3D12_VERTEX_BUFFER_VIEW output =
		{
			.BufferLocation = vertexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UINT>(verticesSize),
			.StrideInBytes = static_cast<UINT>(vertexSize),
		};

		return *Reinterpret(&output);
	}

	IndexBufferView D3D12Helper::CreateIndexBufferView(const GraphicsBuffer& indexBuffer, int indicesSize, Format indexFormat)
	{
		D3D12_INDEX_BUFFER_VIEW output =
		{
			.BufferLocation = indexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UINT>(indicesSize),
			.Format = static_cast<DXGI_FORMAT>(indexFormat),
		};

		return *Reinterpret(&output);
	}

	void D3D12Helper::CreateCBVAndRegistToDescriptorHeap(
		const Device& device, const DescriptorHeap& descriptorHeap, const GraphicsBuffer& graphicsBuffer, int index)
	{
		const D3D12_CONSTANT_BUFFER_VIEW_DESC desc =
		{
			.BufferLocation = graphicsBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = static_cast<UINT>(graphicsBuffer->GetDesc().Width)
		};

		const DescriptorHeapHandleAtCPU handleCBV =
			CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
				device, descriptorHeap,
				DescriptorHeapType::CBV_SRV_UAV,
				index
			);

		device->CreateConstantBufferView(
			&desc,
			*Reinterpret(&handleCBV)
		);
	}

	void D3D12Helper::CreateSRVAndRegistToDescriptorHeap(
		const Device& device, const DescriptorHeap& descriptorHeap, const GraphicsBuffer& graphicsBuffer, int index,
		const Texture& textureAsMetadata)
	{
		const bool isArray = textureAsMetadata.sliceCount > 1;
		const D3D12_SHADER_RESOURCE_VIEW_DESC desc = isArray ?
			D3D12_SHADER_RESOURCE_VIEW_DESC
		{
			// 2次元テクスチャ配列

			.Format = static_cast<DXGI_FORMAT>(textureAsMetadata.format),
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY, // 2Dテクスチャ配列
			// デフォルト : テクセル値をそのまま、フォーマット・順序を変えずマッピングする
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,

			// union
			.Texture2DArray =
			{
				.MostDetailedMip = 0, // 規定値
				.MipLevels = 1, // ミップマップは使わない
				.FirstArraySlice = 0,
				.ArraySize = static_cast<UINT>(textureAsMetadata.sliceCount),
				.PlaneSlice = 0, // 規定値
				.ResourceMinLODClamp = 0.0f // 規定値
			}
		}:
		D3D12_SHADER_RESOURCE_VIEW_DESC
		{
			// 2次元テクスチャ単体

			.Format = static_cast<DXGI_FORMAT>(textureAsMetadata.format),
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D, // 2Dテクスチャ
			// デフォルト : テクセル値をそのまま、フォーマット・順序を変えずマッピングする
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,

			// union
			.Texture2D =
			{
				.MostDetailedMip = 0, // 規定値
				.MipLevels = 1, // ミップマップは使わない
				.PlaneSlice = 0, // 規定値
				.ResourceMinLODClamp = 0.0f // 規定値
			}
		};

		const DescriptorHeapHandleAtCPU handleSRV =
			CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
				device, descriptorHeap,
				DescriptorHeapType::CBV_SRV_UAV,
				index
			);

		device->CreateShaderResourceView(
			graphicsBuffer.Ptr,
			&desc,
			*Reinterpret(&handleSRV)
		);
	}

	DescriptorHeapHandleAtCPU D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
		const Device& device, const DescriptorHeap& descriptorHeap, DescriptorHeapType descriptorHeapType, int index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += index * device->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(descriptorHeapType));
		return *Reinterpret(&handle);
	}

	DescriptorHeapHandleAtGPU D3D12Helper::CreateDescriptorHeapHandleAtGPUIndicatingDescriptorByIndex(
		const Device& device, const DescriptorHeap& descriptorHeap, DescriptorHeapType descriptorHeapType, int index)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += index * device->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(descriptorHeapType));
		return *Reinterpret(&handle);
	}

	bool D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(
		const GraphicsBuffer& graphicsBuffer, const void* dataBegin, int dataSize, bool unmapOnEnd, void** outBufferVirtualPtr)
	{
		if (outBufferVirtualPtr)
			*outBufferVirtualPtr = nullptr;

		void* bufferVirtualPtr = nullptr;
		if (FAILED(graphicsBuffer->Map(
			0, // リソース配列やミップマップなどではないので、0でOK
			nullptr, // GraphicsBuffer の全範囲を対象にする
			&bufferVirtualPtr
		)))
		{
			if (unmapOnEnd)
				graphicsBuffer->Unmap(0, nullptr);
			return false;
		}

		std::memcpy(bufferVirtualPtr, dataBegin, static_cast<std::size_t>(dataSize));
		if (unmapOnEnd)
			graphicsBuffer->Unmap(0, nullptr);
		else
			if (outBufferVirtualPtr)
				*outBufferVirtualPtr = bufferVirtualPtr;
		return true;
	}

	bool D3D12Helper::CommandCopyDataFromCPUToGPUThroughGraphicsBufferTexture2D(
		const CommandList& commandList,
		const GraphicsBuffer& textureCopyIntermediateBuffer, const GraphicsBuffer& textureBuffer, const Texture& texture)
	{
		if (texture.textureType != GraphicsBufferType::Texture2D)
		{
			return false;
		}

		const int alignedRowSize = GetAlignmentedSize(texture.rowSize, Texture::RowSizeAlignment);

		// 中間バッファ(1次元) にマップ
		{
			void* bufferVirtualPtr = nullptr;
			if (FAILED(textureCopyIntermediateBuffer->Map(
				0, // 1次元バッファなので、これで良い
				nullptr, // GraphicsBuffer の全範囲を対象にする
				&bufferVirtualPtr
			)))
			{
				textureCopyIntermediateBuffer->Unmap(0, nullptr);
				return false;
			}

			// バッファ間でコピー
			{
				std::uint8_t* dstBase = static_cast<std::uint8_t*>(bufferVirtualPtr);

				for (int sliceIndex = 0; sliceIndex < texture.sliceCount; ++sliceIndex)
				{
					std::uint8_t* src = const_cast<std::uint8_t*>(texture.data.data())
						+ sliceIndex * texture.sliceSize;
					std::uint8_t* dst = dstBase
						+ sliceIndex * alignedRowSize * texture.height;

					if (!src || !dst)
					{
						textureCopyIntermediateBuffer->Unmap(0, nullptr);
						return false;
					}

					// RowPitch のアラインメントがあって、バッファのサイズが異なるので、1行ごとにコピーするようにする
					for (int y = 0; y < texture.height; ++y)
					{
						std::memcpy(dst, src, texture.rowSize);
						src += texture.rowSize;
						dst += alignedRowSize;
					}
				}
			}

			textureCopyIntermediateBuffer->Unmap(0, nullptr);
		}

		// 真のテクスチャバッファーにコピー
		{
			for (int sliceIndex = 0; sliceIndex < texture.sliceCount; ++sliceIndex)
			{
				const D3D12_TEXTURE_COPY_LOCATION srcLocation =
				{
					.pResource = textureCopyIntermediateBuffer.Ptr,
					.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, // アップロードバッファーはこっちを指定
					.PlacedFootprint =
					{
						.Offset = static_cast<UINT64>(sliceIndex * alignedRowSize * texture.height),
						.Footprint =
						{
							.Format = static_cast<DXGI_FORMAT>(texture.format),
							.Width = static_cast<UINT>(texture.width),
							.Height = static_cast<UINT>(texture.height),
							.Depth = 1, // 2Dテクスチャなので...
							.RowPitch = static_cast<UINT>(alignedRowSize),
						}
					}
				};

				const D3D12_TEXTURE_COPY_LOCATION dstLocation =
				{
					.pResource = textureBuffer.Ptr,
					.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, // コピー先バッファーはこっちを指定
					.SubresourceIndex = static_cast<UINT>(sliceIndex)
				};

				// コマンドを発行
				commandList->CopyTextureRegion(
					&dstLocation,
					0, // コピー先の開始オフセットは0でOK
					0, // コピー先の開始オフセットは0でOK
					0, // コピー先の開始オフセットは0でOK
					&srcLocation,
					nullptr // コピー元の全領域をコピーするので、nullptrでOK
				);
			}
		}

		return true;
	}

	bool D3D12Helper::ClearCommandAllocatorAndList(const CommandAllocator& commandAllocator, const CommandList& commandList)
	{
		if (FAILED(commandAllocator->Reset())) return false;
		if (FAILED(commandList->Reset(commandAllocator.Ptr, nullptr))) return false;
		return true;
	}

	int D3D12Helper::GetCurrentBackBufferIndex(const SwapChain& swapChain)
	{
		return swapChain->GetCurrentBackBufferIndex();
	}

	GraphicsBuffer D3D12Helper::GetBufferByIndex(const SwapChain& swapChain, int index)
	{
		ID3D12Resource* ptr = nullptr;
		if (SUCCEEDED(swapChain->GetBuffer(index, IID_PPV_ARGS(&ptr))))
		{
			return GraphicsBuffer(ptr);
		}

		return GraphicsBuffer();
	}

	void D3D12Helper::CommandInvokeResourceBarrierAsTransition(
		const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer,
		GraphicsBufferState before, GraphicsBufferState after, bool useAllSubresources)
	{
		D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES; // 定数
		const D3D12_RESOURCE_BARRIER desc =
		{
			.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, // 状態遷移
			.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE, // 特別なことはしないので、指定しない
			.Transition = // union なので、これじゃないメンバは触らないこと!
			{
				.pResource = GraphicsBuffer.Ptr, // GraphicsBuffer のアドレスをそのまま渡せば良い
				.Subresource = useAllSubresources ? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES : 0, // 全サブリソース or サブリソース0のみ
				.StateBefore = static_cast<D3D12_RESOURCE_STATES>(before),
				.StateAfter = static_cast<D3D12_RESOURCE_STATES>(after),
			},
		};

		// 配列で渡せるが、今回はバリアは1つだけ
		commandList->ResourceBarrier(1, &desc);
	}

	void D3D12Helper::CommandSetRT(const CommandList& commandList,
		const DescriptorHeapHandleAtCPU& rtv, const DescriptorHeapHandleAtCPU& dsv)
	{
		commandList->OMSetRenderTargets(
			1, // 今のところ、一回の描画における RTV は1つのみ
			Reinterpret(const_cast<DescriptorHeapHandleAtCPU*>(&rtv)),
			false, // 規定値
			Reinterpret(const_cast<DescriptorHeapHandleAtCPU*>(&dsv))
		);
	}

	void D3D12Helper::CommandClearRT(
		const CommandList& commandList, const DescriptorHeapHandleAtCPU& rtv, const DescriptorHeapHandleAtCPU& dsv,
		Color rtvClearValue, float dsvClearValue)
	{
		const float clearColorAsArray[4] = { rtvClearValue.r, rtvClearValue.g, rtvClearValue.b, rtvClearValue.a };
		commandList->ClearRenderTargetView(
			*Reinterpret(const_cast<DescriptorHeapHandleAtCPU*>(&rtv)),
			clearColorAsArray,
			0, nullptr // 全範囲をクリアするので、指定しない
		);

		commandList->ClearDepthStencilView(
			*Reinterpret(const_cast<DescriptorHeapHandleAtCPU*>(&dsv)),
			D3D12_CLEAR_FLAG_DEPTH, // 深度値のみクリア
			dsvClearValue,
			0, // ステンシル値はクリアしないので、0でOK
			0, nullptr // 全範囲をクリアするので、指定しない
		);
	}

	void D3D12Helper::CommandSetRootSignature(const CommandList& commandList, const RootSignature& rootSignature)
	{
		commandList->SetGraphicsRootSignature(rootSignature.Ptr);
	}

	void D3D12Helper::CommandSetGraphicsPipelineState(const CommandList& commandList, const PipelineState& graphicsPipelineState)
	{
		commandList->SetPipelineState(graphicsPipelineState.Ptr);
	}

	void D3D12Helper::CommandSetDescriptorHeaps(const CommandList& commandList, const std::vector<DescriptorHeap>& descriptorHeaps)
	{
		std::vector<ID3D12DescriptorHeap*> descriptorHeapsReal = std::vector<ID3D12DescriptorHeap*>(descriptorHeaps.size(), {});
		for (int i = 0; i < static_cast<int>(descriptorHeaps.size()); ++i)
			descriptorHeapsReal[i] = descriptorHeaps[i].Ptr;

		commandList->SetDescriptorHeaps(
			static_cast<UINT>(descriptorHeapsReal.size()),
			descriptorHeapsReal.data()
		);
	}

	void D3D12Helper::CommandLinkDescriptorHeapToRootSignature(
		const CommandList& commandList, const DescriptorHeapHandleAtGPU& firstDescriptor, int rootParameterIndex)
	{
		commandList->SetGraphicsRootDescriptorTable(
			static_cast<UINT>(rootParameterIndex),
			*Reinterpret(&firstDescriptor)
		);
	}

	void D3D12Helper::CommandIASetPrimitiveTopology(const CommandList& commandList, PrimitiveTopology primitiveTopology)
	{
		commandList->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(primitiveTopology));
	}

	void D3D12Helper::CommandIASetVertexBuffer(const CommandList& commandList, const std::vector<VertexBufferView>& vertexBufferViews)
	{
		const VertexBufferView* vertexBufferViewsPtr = vertexBufferViews.data();
		const D3D12_VERTEX_BUFFER_VIEW* d3d12VertexBufferViewsPtr = Reinterpret(const_cast<VertexBufferView*>(vertexBufferViewsPtr));

		commandList->IASetVertexBuffers(
			0, // スロット番号 0
			static_cast<UINT>(vertexBufferViews.size()),
			d3d12VertexBufferViewsPtr
		);
	}

	void D3D12Helper::CommandIASetIndexBuffer(const CommandList& commandList, const IndexBufferView& indexBufferView)
	{
		const D3D12_INDEX_BUFFER_VIEW* d3d12IndexBufferViewPtr = Reinterpret(const_cast<IndexBufferView*>(&indexBufferView));

		commandList->IASetIndexBuffer(d3d12IndexBufferViewPtr);
	}

	void D3D12Helper::CommandRSSetViewportAndScissorRect(const CommandList& commandList, const ViewportScissorRect& viewportScissorRect)
	{
		const auto [viewport, scissorRect] = Construct(viewportScissorRect);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
	}

	void D3D12Helper::CommandDrawInstanced(const CommandList& commandList, int vertexCount)
	{
		commandList->DrawInstanced(
			static_cast<UINT>(vertexCount),
			1, // インスタンス数 (今回はインスタンシングしないので、1でOK)
			0, // 頂点データのオフセット
			0  // インスタンスのオフセット
		);
	}

	void D3D12Helper::CommandDrawIndexedInstanced(const CommandList& commandList, int indexCount)
	{
		commandList->DrawIndexedInstanced(
			static_cast<UINT>(indexCount),
			1, // インスタンス数 (今回はインスタンシングしないので、1でOK)
			0, // インデックスデータのオフセット
			0, // 頂点データのオフセット
			0  // インスタンスのオフセット
		);
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
		if (FAILED(commandQueue->Signal(fence.Ptr, FenceValueAfterGPUEvent)))
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
		return SUCCEEDED(swapChain->Present(1, 0));
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

		if (SUCCEEDED(result))
		{
			outErrorMessage = L"";
			if (errorBlob) errorBlob->Release();
			return Blob(blob);
		}

		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			outErrorMessage = L"Shader file not found.";
			if (errorBlob) errorBlob->Release();
			return Blob();
		}

		outErrorMessage = FetchErrorMessageFromErrorBlob(Blob(errorBlob));
		if (errorBlob) errorBlob->Release();
		return Blob();
	}

	bool D3D12Helper::CompileShaderFile_VS_PS(
		const std::wstring& path,
		const std::string& entryFuncVS, const std::string& entryFuncPS,
		Blob& outVS, Blob& outPS,
		std::wstring& outErrorMessage
	)
	{
		// 一通り処理を試行

		Blob shaderVS, shaderPS;
		std::wstring errorMessage, errorMessageTmp;

		shaderVS = D3D12Helper::CompileShaderFile(path, entryFuncVS, ShaderTargetVS, errorMessageTmp);
		if (!shaderVS) errorMessage += errorMessageTmp + L"\n";

		shaderPS = D3D12Helper::CompileShaderFile(path, entryFuncPS, ShaderTargetPS, errorMessageTmp);
		if (!shaderPS) errorMessage += errorMessageTmp + L"\n";

		if (!shaderVS || !shaderPS)
		{
			outVS = Blob();
			outPS = Blob();
			outErrorMessage = errorMessage;

			return false;
		}

		outVS = shaderVS;
		outPS = shaderPS;
		outErrorMessage = L"";

		return true;
	}

#ifdef _DEBUG
	bool D3D12Helper::EnableDebugLayer()
	{
		ID3D12Debug* debugLayer = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
		{
			debugLayer->EnableDebugLayer();
			debugLayer->Release();

			return true;
		}

		return false;
	}
#endif

	D3D12_DESCRIPTOR_RANGE Construct(const RootParameter::DescriptorRange& descriptorRange)
	{
		return
		{
			.RangeType = static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(descriptorRange.type),
			.NumDescriptors = static_cast<UINT>(descriptorRange.amount),
			.BaseShaderRegister = static_cast<UINT>(descriptorRange.shaderRegister),
			.RegisterSpace = 0, // つじつまを合わせる用 (0でOK)
			.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND // 自動で割り当て
		};
	}

	D3D12_STATIC_SAMPLER_DESC Construct(const SamplerConfig& samplerConfig)
	{
		return
		{
			.Filter = static_cast<D3D12_FILTER>(samplerConfig.filter),
			.AddressU = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(samplerConfig.addressingMode),
			.AddressV = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(samplerConfig.addressingMode),
			.AddressW = static_cast<D3D12_TEXTURE_ADDRESS_MODE>(samplerConfig.addressingMode), // 3Dテクスチャだと関与してくる
			.MipLODBias = 0.0f, // 規定値
			.MaxAnisotropy = 0, // 規定値
			.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER, // リサンプリングしない
			.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK, // 端の色 規定値
			.MinLOD = 0.0f, // ミップマップの下限
			.MaxLOD = D3D12_FLOAT32_MAX, // ミップマップの上限
			.ShaderRegister = static_cast<UINT>(samplerConfig.shaderRegister),
			.RegisterSpace = 0, // 規定値
			.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(samplerConfig.shaderVisibility)
		};
	}

	D3D12_INPUT_ELEMENT_DESC Construct(const VertexLayout& vertexLayout)
	{
		return
		{
			.SemanticName = vertexLayout.SemanticName,
			.SemanticIndex = 0, // 同じセマンティクス名が複数ある場合のインデックス (0でOK)
			.Format = static_cast<DXGI_FORMAT>(vertexLayout.Format),
			.InputSlot = 0, // インターリーブ なので、0番スロットだけ使えばOK
			.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT, // 頂点バッファのメンバは連続しているので、それらのアドレスは自動で計算してもらう
			.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // 1頂点ごとにこのレイアウトが入っている
			.InstanceDataStepRate = 0, // インスタンシングではないので、データは使いまわさない
		};
	}

	std::pair<D3D12_VIEWPORT, D3D12_RECT> Construct(const ViewportScissorRect& viewportScissorRect)
	{
		const D3D12_VIEWPORT viewport =
		{
			.TopLeftX = static_cast<FLOAT>(viewportScissorRect.minX),
			.TopLeftY = static_cast<FLOAT>(viewportScissorRect.minY),
			.Width = static_cast<FLOAT>(viewportScissorRect.maxX - viewportScissorRect.minX),
			.Height = static_cast<FLOAT>(viewportScissorRect.maxY - viewportScissorRect.minY),
			.MinDepth = 0.0f,
			.MaxDepth = 1.0f,
		};

		const D3D12_RECT scissorRect =
		{
			.left = viewportScissorRect.minX,
			.top = viewportScissorRect.minY,
			.right = viewportScissorRect.maxX,
			.bottom = viewportScissorRect.maxY,
		};

		return { viewport, scissorRect };
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* Reinterpret(const DescriptorHeapHandleAtCPU* value)
	{
		return reinterpret_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(const_cast<DescriptorHeapHandleAtCPU*>(value));
	}

	DescriptorHeapHandleAtCPU* Reinterpret(const D3D12_CPU_DESCRIPTOR_HANDLE* value)
	{
		return reinterpret_cast<DescriptorHeapHandleAtCPU*>(const_cast<D3D12_CPU_DESCRIPTOR_HANDLE*>(value));
	}

	D3D12_GPU_DESCRIPTOR_HANDLE* Reinterpret(const DescriptorHeapHandleAtGPU* value)
	{
		return reinterpret_cast<D3D12_GPU_DESCRIPTOR_HANDLE*>(const_cast<DescriptorHeapHandleAtGPU*>(value));
	}

	DescriptorHeapHandleAtGPU* Reinterpret(const D3D12_GPU_DESCRIPTOR_HANDLE* value)
	{
		return reinterpret_cast<DescriptorHeapHandleAtGPU*>(const_cast<D3D12_GPU_DESCRIPTOR_HANDLE*>(value));
	}

	D3D12_VERTEX_BUFFER_VIEW* Reinterpret(const VertexBufferView* value)
	{
		return reinterpret_cast<D3D12_VERTEX_BUFFER_VIEW*>(const_cast<VertexBufferView*>(value));
	}

	VertexBufferView* Reinterpret(const D3D12_VERTEX_BUFFER_VIEW* value)
	{
		return reinterpret_cast<VertexBufferView*>(const_cast<D3D12_VERTEX_BUFFER_VIEW*>(value));
	}

	D3D12_INDEX_BUFFER_VIEW* Reinterpret(const IndexBufferView* value)
	{
		return reinterpret_cast<D3D12_INDEX_BUFFER_VIEW*>(const_cast<IndexBufferView*>(value));
	}

	IndexBufferView* Reinterpret(const D3D12_INDEX_BUFFER_VIEW* value)
	{
		return reinterpret_cast<IndexBufferView*>(const_cast<D3D12_INDEX_BUFFER_VIEW*>(value));
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
		if (SUCCEEDED(swapChain->GetDesc(&desc)))
		{
			return desc.BufferCount;
		}

		return -1;
	}
}
