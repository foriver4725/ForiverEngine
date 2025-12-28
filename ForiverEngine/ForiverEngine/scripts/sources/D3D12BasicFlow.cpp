#include "../headers/D3D12BasicFlow.h"

namespace ForiverEngine
{
	std::tuple<bool, std::wstring, std::tuple<Factory, Device, CommandAllocator, CommandList, CommandQueue, SwapChain>>
		D3D12BasicFlow::CreateStandardObjects_Impl(
			HWND hwnd,
			int windowWidth,
			int windowHeight
		)
	{
		Factory factory = Factory();
		Device device = Device();
		CommandAllocator commandAllocater = CommandAllocator();
		CommandList commandList = CommandList();
		CommandQueue commandQueue = CommandQueue();
		SwapChain swapChain = SwapChain();

#define RETURN_FALSE(errorMessage) \
    return { false, errorMessage, { factory, device, commandAllocater, commandList, commandQueue, swapChain } };
#define RETURN_TRUE() \
	return { true, L"", { factory, device, commandAllocater, commandList, commandQueue, swapChain } };

		if (!(factory = D3D12Helper::CreateFactory()))
			RETURN_FALSE(L"Factory の作成に失敗しました");

		if (!(device = D3D12Helper::CreateDevice(factory)))
			RETURN_FALSE(L"Device の作成に失敗しました");

		if (!(commandAllocater = D3D12Helper::CreateCommandAllocator(device)))
			RETURN_FALSE(L"CommandAllocater の作成に失敗しました");

		if (!(commandList = D3D12Helper::CreateCommandList(device, commandAllocater)))
			RETURN_FALSE(L"CommandList の作成に失敗しました");

		if (!(commandQueue = D3D12Helper::CreateCommandQueue(device)))
			RETURN_FALSE(L"CommandQueue の作成に失敗しました");

		if (!(swapChain = D3D12Helper::CreateSwapChain(factory, commandQueue, hwnd, windowWidth, windowHeight)))
			RETURN_FALSE(L"SwapChain の作成に失敗しました");

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<VertexBufferView, IndexBufferView>>
		D3D12BasicFlow::CreateVertexAndIndexBufferViews_Impl(
			const Device& device,
			const Mesh& mesh
		)
	{
		VertexBufferView vertexBufferView = VertexBufferView();
		IndexBufferView indexBufferView = IndexBufferView();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { vertexBufferView, indexBufferView } };
#define RETURN_TRUE() \
	return { true, L"", { vertexBufferView, indexBufferView } };

		const std::vector<VertexData>& vertices = mesh.vertices;                 // メッシュのプロパティ
		const VertexData* verticesPtr = vertices.data();                         // 先頭ポインタ
		const int vertexSize = static_cast<int>(sizeof(vertices[0]));            // 要素1つ分のメモリサイズ
		const int verticesSize = static_cast<int>(vertices.size() * vertexSize); // 全体のメモリサイズ

		const std::vector<std::uint32_t>& indices = mesh.indices;                // メッシュのプロパティ
		const std::uint32_t* indicesPtr = indices.data();                        // 先頭ポインタ
		const int indexSize = static_cast<int>(sizeof(indices[0]));              // 要素1つ分のメモリサイズ
		const int indicesSize = static_cast<int>(indices.size() * indexSize);    // 全体のメモリサイズ

		GraphicsBuffer vertexBuffer = GraphicsBuffer();
		GraphicsBuffer indexBuffer = GraphicsBuffer();

		if (!(vertexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, verticesSize, true)))
			RETURN_FALSE(L"頂点バッファーの作成に失敗しました");
		if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(vertexBuffer, static_cast<const void*>(verticesPtr), verticesSize))
			RETURN_FALSE(L"頂点バッファーを GPU 側にコピーすることに失敗しました");
		vertexBufferView = D3D12Helper::CreateVertexBufferView(vertexBuffer, verticesSize, vertexSize);

		if (!(indexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, indicesSize, true)))
			RETURN_FALSE(L"インデックスバッファーの作成に失敗しました");
		if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(indexBuffer, static_cast<const void*>(indicesPtr), indicesSize))
			RETURN_FALSE(L"インデックスバッファーを GPU 側にコピーすることに失敗しました");
		indexBufferView = D3D12Helper::CreateIndexBufferView(indexBuffer, indicesSize, Format::R_U32);

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<VertexBufferView, IndexBufferView>>
		D3D12BasicFlow::CreateVertexAndIndexBufferViewsPP_Impl(
			const Device& device,
			const MeshPP& mesh
		)
	{
		VertexBufferView vertexBufferView = VertexBufferView();
		IndexBufferView indexBufferView = IndexBufferView();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { vertexBufferView, indexBufferView } };
#define RETURN_TRUE() \
	return { true, L"", { vertexBufferView, indexBufferView } };

		const std::vector<VertexDataPP>& vertices = mesh.vertices;               // メッシュのプロパティ
		const VertexDataPP* verticesPtr = vertices.data();                       // 先頭ポインタ
		const int vertexSize = static_cast<int>(sizeof(vertices[0]));            // 要素1つ分のメモリサイズ
		const int verticesSize = static_cast<int>(vertices.size() * vertexSize); // 全体のメモリサイズ

		const std::vector<std::uint32_t>& indices = mesh.indices;                // メッシュのプロパティ
		const std::uint32_t* indicesPtr = indices.data();                        // 先頭ポインタ
		const int indexSize = static_cast<int>(sizeof(indices[0]));              // 要素1つ分のメモリサイズ
		const int indicesSize = static_cast<int>(indices.size() * indexSize);    // 全体のメモリサイズ

		GraphicsBuffer vertexBuffer = GraphicsBuffer();
		GraphicsBuffer indexBuffer = GraphicsBuffer();

		if (!(vertexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, verticesSize, true)))
			RETURN_FALSE(L"頂点バッファーの作成に失敗しました");
		if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(vertexBuffer, static_cast<const void*>(verticesPtr), verticesSize))
			RETURN_FALSE(L"頂点バッファーを GPU 側にコピーすることに失敗しました");
		vertexBufferView = D3D12Helper::CreateVertexBufferView(vertexBuffer, verticesSize, vertexSize);

		if (!(indexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, indicesSize, true)))
			RETURN_FALSE(L"インデックスバッファーの作成に失敗しました");
		if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(indexBuffer, static_cast<const void*>(indicesPtr), indicesSize))
			RETURN_FALSE(L"インデックスバッファーを GPU 側にコピーすることに失敗しました");
		indexBufferView = D3D12Helper::CreateIndexBufferView(indexBuffer, indicesSize, Format::R_U32);

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<Blob, Blob>>
		D3D12BasicFlow::CompileShader_VS_PS_Impl(
			const std::string& path
		)
	{
		Blob vs = Blob();
		Blob ps = Blob();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { vs, ps } };
#define RETURN_TRUE() \
	return { true, L"", { vs, ps } };

		std::wstring errorMessage = L"";

		if (!D3D12Helper::CompileShaderFile_VS_PS(
			StringUtils::UTF8ToUTF16(path),
			ShaderEntryFuncVS, ShaderEntryFuncPS,
			vs, ps,
			errorMessage
		))
			RETURN_FALSE(errorMessage.c_str());

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<RootSignature, PipelineState>>
		D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState_Impl(
			const Device& device,
			const RootParameter& rootParameter,
			const SamplerConfig& samplerConfig,
			const Blob& shaderVS,
			const Blob& shaderPS,
			const std::vector<VertexLayout>& vertexLayouts,
			FillMode fillMode,
			CullMode cullMode
		)
	{
		RootSignature rootSignature = RootSignature();
		PipelineState graphicsPipelineState = PipelineState();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { rootSignature, graphicsPipelineState } };
#define RETURN_TRUE() \
	return { true, L"", { rootSignature, graphicsPipelineState } };

		std::wstring errorMessage = L"";

		rootSignature = D3D12Helper::CreateRootSignature(device, rootParameter, samplerConfig, errorMessage);
		if (!rootSignature)
			RETURN_FALSE(errorMessage.c_str());

		graphicsPipelineState = D3D12Helper::CreateGraphicsPipelineState(
			device, rootSignature, shaderVS, shaderPS, vertexLayouts, fillMode, cullMode);
		if (!graphicsPipelineState)
			RETURN_FALSE(L"GraphicsPipelineState の作成に失敗しました");

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<GraphicsBuffer, Texture>>
		D3D12BasicFlow::InitSRVBuffer_Impl(
			const Device& device,
			const CommandList& commandList,
			const CommandQueue& commandQueue,
			const std::vector<std::string>& paths
		)
	{
		Texture textureArray = Texture();
		GraphicsBuffer textureArrayBuffer = GraphicsBuffer();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { textureArrayBuffer, textureArray } };
#define RETURN_TRUE() \
	return { true, L"", { textureArrayBuffer, textureArray } };

		textureArray = AssetLoader::LoadTextureArray(paths);
		if (!textureArray.IsValid())
			RETURN_FALSE(L"テクスチャ群のロードに失敗しました");

		textureArrayBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, textureArray);
		if (!textureArrayBuffer)
			RETURN_FALSE(L"テクスチャ配列バッファの作成に失敗しました");

		D3D12BasicFlow::UploadTextureToGPU(commandList, commandQueue, device, textureArrayBuffer, textureArray);

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<DescriptorHeap>>
		D3D12BasicFlow::InitDescriptorHeapBasic_Impl(
			const Device& device,
			const std::vector<GraphicsBuffer>& cbvBuffers,
			const std::vector<std::tuple<GraphicsBuffer, Texture>>& srvBuffers
		)
	{
		const int cbvBufferCount = static_cast<int>(cbvBuffers.size());
		const int srvBufferCount = static_cast<int>(srvBuffers.size());
		const int bufferTotalCount = cbvBufferCount + srvBufferCount;

		const DescriptorHeap descriptorHeapBasic =
			D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::CBV_SRV_UAV, bufferTotalCount, true);
		if (!descriptorHeapBasic)
			return { false, L"DescriptorHeap (CBV_SRV_UAV) の作成に失敗しました", { DescriptorHeap() } };

		for (int i = 0; i < cbvBufferCount; ++i)
		{
			const auto& buffer = cbvBuffers[i];
			D3D12Helper::CreateCBVAndRegistToDescriptorHeap(device, descriptorHeapBasic, buffer, i);
		}
		for (int i = 0; i < srvBufferCount; ++i)
		{
			const auto& [buffer, textureAsMetadata] = srvBuffers[i];
			D3D12Helper::CreateSRVAndRegistToDescriptorHeap(device, descriptorHeapBasic, buffer, cbvBufferCount + i, textureAsMetadata);
		}

		return { true, L"", { descriptorHeapBasic } };
	}

	std::tuple<bool, std::wstring, std::tuple<std::function<GraphicsBuffer(int)>, std::function<DescriptorHeapHandleAtCPU(int)>>>
		D3D12BasicFlow::InitRTV_Impl(
			const Device& device,
			const SwapChain& swapChain,
			int amount,
			bool sRGB
		)
	{
		std::function<GraphicsBuffer(int)> bufferGetter = std::function<GraphicsBuffer(int)>();
		std::function<DescriptorHeapHandleAtCPU(int)> viewGetter = std::function<DescriptorHeapHandleAtCPU(int)>();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { bufferGetter, viewGetter } };
#define RETURN_TRUE() \
	return { true, L"", { bufferGetter, viewGetter } };

		const DescriptorHeap descriptorHeapRTV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::RTV, amount, false);
		if (!descriptorHeapRTV)
			RETURN_FALSE(L"DescriptorHeap (RTV) の作成に失敗しました");

		if (!D3D12Helper::CreateRenderTargetViews(device, descriptorHeapRTV, swapChain, sRGB))
			RETURN_FALSE(L"RenderTargetView を作成できない RenderTargetBuffer がありました");

		bufferGetter = [swapChain](int i) { return D3D12Helper::GetBufferByIndex(swapChain, i); };
		viewGetter = [device, descriptorHeapRTV](int i) {
			return D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
				device, descriptorHeapRTV, DescriptorHeapType::RTV, i);
			};

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<DescriptorHeapHandleAtCPU>>
		D3D12BasicFlow::InitRTVPP_Impl(
			const Device& device,
			const GraphicsBuffer& rt
		)
	{
		DescriptorHeapHandleAtCPU rtv = DescriptorHeapHandleAtCPU();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { rtv } };
#define RETURN_TRUE() \
	return { true, L"", { rtv } };

		const DescriptorHeap descriptorHeapRTV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::RTV, 1, false);
		if (!descriptorHeapRTV)
			RETURN_FALSE(L"DescriptorHeap (RTV) の作成に失敗しました");

		D3D12Helper::CreateRenderTargetViewPP(device, descriptorHeapRTV, rt, 0);

		rtv = D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
			device, descriptorHeapRTV, DescriptorHeapType::RTV, 0
		);

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<DescriptorHeapHandleAtCPU>>
		D3D12BasicFlow::InitDSV_Impl(
			const Device& device,
			int width,
			int height,
			float depthClearValue
		)
	{
		DescriptorHeapHandleAtCPU dsv = DescriptorHeapHandleAtCPU();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { dsv } };
#define RETURN_TRUE() \
	return { true, L"", { dsv } };

		const GraphicsBuffer depthBuffer = D3D12Helper::CreateGraphicsBufferTexture2DAsDepthBuffer(device, width, height, depthClearValue);
		if (!depthBuffer)
			RETURN_FALSE(L"DepthBuffer の作成に失敗しました");

		const DescriptorHeap descriptorHeapDSV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::DSV, 1, false);
		if (!descriptorHeapDSV)
			RETURN_FALSE(L"DescriptorHeap (DSV) の作成に失敗しました");

		D3D12Helper::CreateDepthStencilView(device, descriptorHeapDSV, depthBuffer);

		dsv = D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
			device, descriptorHeapDSV, DescriptorHeapType::DSV, 0);

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring>
		D3D12BasicFlow::CommandCloseAndWaitForCompletion_Impl(
			const CommandList& commandList,
			const CommandQueue& commandQueue,
			const Device& device
		)
	{
		D3D12Helper::CommandClose(commandList);

		D3D12Helper::ExecuteCommands(commandQueue, commandList);

		if (!D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue))
			return { false, L"GPU の処理待ち受けに失敗しました" };

		return { true, L"" };
	}

	std::tuple<bool, std::wstring>
		D3D12BasicFlow::UploadTextureToGPU_Impl(
			const CommandList& commandList,
			const CommandQueue& commandQueue,
			const Device& device,
			const GraphicsBuffer& textureBuffer,
			const Texture& textureAsMetadata
		)
	{
		const GraphicsBuffer intermediateBuffer = D3D12Helper::CreateGraphicsBuffer1D(
			device,
			static_cast<int>(
				GetAlignmentedSize(textureAsMetadata.rowSize, Texture::RowSizeAlignment)
				* textureAsMetadata.height
				* textureAsMetadata.sliceCount
				),
			true
		);
		if (!intermediateBuffer)
			return { false, L"テクスチャ転送用中間バッファの作成に失敗しました" };

		if (!D3D12Helper::CommandCopyDataFromCPUToGPUThroughGraphicsBufferTexture2D(
			commandList, intermediateBuffer, textureBuffer, textureAsMetadata))
			return { false, L"テクスチャデータを GPU 側にコピーするコマンドの発行に失敗しました" };

		D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, textureBuffer,
			GraphicsBufferState::CopyDestination, GraphicsBufferState::PixelShaderResource, false);

		D3D12BasicFlow::CommandCloseAndWaitForCompletion(commandList, commandQueue, device);

		return { true, L"" };
	}

	std::tuple<bool, std::wstring>
		D3D12BasicFlow::CommandBasicLoop_Impl(
			// 基本オブジェクト
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Device& device,
			// パイプライン関連
			const RootSignature& rootSignature, const PipelineState& graphicsPipelineState, const GraphicsBuffer& rt,
			// Descriptor
			const DescriptorHeapHandleAtCPU& rtv, const DescriptorHeapHandleAtCPU& dsv,
			const DescriptorHeap& descriptorHeapBasic,
			const std::vector<VertexBufferView>& vertexBufferViewArray,
			const std::vector<IndexBufferView>& indexBufferViewArray,
			// 数値情報
			const ViewportScissorRect& viewportScissorRect, PrimitiveTopology primitiveTopology,
			Color rtvClearColor, float depthClearValue,
			// ドローコール関連
			const std::vector<int>& indexTotalCountArray
		)
	{
		// ドローコール数を取得
		// 要素数が等しいかも、ついでにチェック
		const std::uint32_t drawCount = static_cast<std::uint32_t>(indexTotalCountArray.size());
		if (drawCount != static_cast<std::uint32_t>(vertexBufferViewArray.size()))
			return { false, L"頂点バッファビューの数と、ドローコール数が一致しません" };
		if (drawCount != static_cast<std::uint32_t>(indexBufferViewArray.size()))
			return { false, L"インデックスバッファビューの数と、ドローコール数が一致しません" };

		D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, rt,
			GraphicsBufferState::Present, GraphicsBufferState::RenderTarget, false);
		{
			D3D12Helper::CommandSetRT(commandList, rtv, dsv);
			D3D12Helper::CommandClearRT(commandList, rtv, dsv, rtvClearColor, depthClearValue);

			D3D12Helper::CommandSetRootSignature(commandList, rootSignature);
			D3D12Helper::CommandSetGraphicsPipelineState(commandList, graphicsPipelineState);
			D3D12Helper::CommandSetDescriptorHeaps(commandList, { descriptorHeapBasic });

			// ルートパラメーターは1つだけなので、インデックス0にリンクすれば良い
			D3D12Helper::CommandLinkDescriptorHeapToRootSignature(
				commandList,
				D3D12Helper::CreateDescriptorHeapHandleAtGPUIndicatingDescriptorByIndex(
					device, descriptorHeapBasic, DescriptorHeapType::CBV_SRV_UAV, 0),
				0
			);

			D3D12Helper::CommandIASetPrimitiveTopology(commandList, primitiveTopology);
			D3D12Helper::CommandRSSetViewportAndScissorRect(commandList, viewportScissorRect);

			// ドローコール分ループ
			for (std::uint32_t i = 0; i < drawCount; ++i)
			{
				D3D12Helper::CommandIASetVertexBuffer(commandList, { vertexBufferViewArray[i] });
				D3D12Helper::CommandIASetIndexBuffer(commandList, indexBufferViewArray[i]);

				D3D12Helper::CommandDrawIndexedInstanced(commandList, indexTotalCountArray[i]);
			}
		}
		D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, rt,
			GraphicsBufferState::RenderTarget, GraphicsBufferState::Present, false);

		D3D12BasicFlow::CommandCloseAndWaitForCompletion(commandList, commandQueue, device);
		// コマンドを実行し終わってから、クリアする
		if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocator, commandList))
			return { false, L"CommandAllocator, CommandList のクリアに失敗しました" };

		return { true, L"" };
	}

	std::tuple<bool, std::wstring, std::tuple<Matrix4x4>>
		D3D12BasicFlow::CalculateMVPMatrix_Impl(
			const Transform& transform,
			const CameraTransform& cameraTransform
		)
	{
		const Matrix4x4 m = transform.CalculateModelMatrix();
		const Matrix4x4 v = cameraTransform.CalculateViewMatrix();
		const Matrix4x4 p = cameraTransform.CalculateProjectionMatrix();

		const Matrix4x4 mvp = p * v * m;

		return { true, L"", { mvp } };
	}
}
