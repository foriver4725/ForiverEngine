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
			const std::vector<VertexData>& vertices,
			const std::vector<std::uint16_t>& indices
		)
	{
		VertexBufferView vertexBufferView = VertexBufferView();
		IndexBufferView indexBufferView = IndexBufferView();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { vertexBufferView, indexBufferView } };
#define RETURN_TRUE() \
	return { true, L"", { vertexBufferView, indexBufferView } };

		const VertexData* verticesPtr = vertices.data();                         // 先頭ポインタ
		const int vertexSize = static_cast<int>(sizeof(vertices[0]));            // 要素1つ分のメモリサイズ
		const int verticesSize = static_cast<int>(vertices.size() * vertexSize); // 全体のメモリサイズ

		const std::uint16_t* indicesPtr = indices.data();                        // 先頭ポインタ
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
		indexBufferView = D3D12Helper::CreateIndexBufferView(indexBuffer, indicesSize, Format::R_U16);

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<GraphicsBuffer, DescriptorHeapHandleAtCPU>>
		D3D12BasicFlow::GetCurrentBackBufferAndView_Impl(
			const Device& device,
			const SwapChain& swapChain,
			const DescriptorHeap& descriptorHeapRTV
		)
	{
		GraphicsBuffer currentBackBuffer = GraphicsBuffer();
		DescriptorHeapHandleAtCPU  currentBackBufferRTV = DescriptorHeapHandleAtCPU();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { currentBackBuffer, currentBackBufferRTV } };
#define RETURN_TRUE() \
	return { true, L"", { currentBackBuffer, currentBackBufferRTV } };

		const int currentBackBufferIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);

		currentBackBuffer = D3D12Helper::GetBufferByIndex(swapChain, currentBackBufferIndex);
		if (!currentBackBuffer)
			RETURN_FALSE(L"SwapChain から現在バックにある GraphicsBuffer を取得することに失敗しました");

		currentBackBufferRTV = D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
			device, descriptorHeapRTV, DescriptorHeapType::RTV, currentBackBufferIndex);

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

	Matrix4x4 D3D12BasicFlow::CalculateMVPMatrix(const Transform& transform, const CameraTransform& cameraTransform)
	{
		const Matrix4x4 m = transform.CalculateModelMatrix();
		const Matrix4x4 v = cameraTransform.CalculateViewMatrix();
		const Matrix4x4 p = cameraTransform.CalculateProjectionMatrix();

		return p * v * m;
	}
}
