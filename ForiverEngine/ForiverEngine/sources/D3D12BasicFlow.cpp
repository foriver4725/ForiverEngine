#include "../headers/D3D12BasicFlow.h"

namespace ForiverEngine
{
	std::tuple<bool, std::wstring, std::tuple<Factory, Device, CommandAllocator, CommandList, CommandQueue, SwapChain, DescriptorHeap>>
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
		DescriptorHeap descriptorHeapRTV = DescriptorHeap();

#define RETURN_FALSE(errorMessage) \
    return { false, errorMessage, { factory, device, commandAllocater, commandList, commandQueue, swapChain, descriptorHeapRTV } };
#define RETURN_TRUE() \
	return { true, L"", { factory, device, commandAllocater, commandList, commandQueue, swapChain, descriptorHeapRTV } };

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

		if (!(descriptorHeapRTV = D3D12Helper::CreateDescriptorHeapRTV(device)))
			RETURN_FALSE(L"DescriptorHeap (RTV) の作成に失敗しました");

		if (!D3D12Helper::LinkDescriptorHeapRTVToSwapChain(device, descriptorHeapRTV, swapChain))
			RETURN_FALSE(L"DescriptorHeap (RTV) を SwapChain に関連付けることに失敗しました");

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}

	std::tuple<bool, std::wstring, std::tuple<VertexBufferView, IndexBufferView>>
		D3D12BasicFlow::CreateVertexAndIndexBufferViews_Impl(
			const Device& device,
			const std::vector<DirectX::XMFLOAT4>& vertices,
			const std::vector<std::uint16_t>& indices
		)
	{
		VertexBufferView vertexBufferView = VertexBufferView();
		IndexBufferView indexBufferView = IndexBufferView();

#define RETURN_FALSE(errorMessage) \
	return { false, errorMessage, { vertexBufferView, indexBufferView } };
#define RETURN_TRUE() \
	return { true, L"", { vertexBufferView, indexBufferView } };

		DirectX::XMFLOAT4* verticesPtr = const_cast<DirectX::XMFLOAT4*>(vertices.data()); // 先頭ポインタ
		const int vertexSize = static_cast<int>(sizeof(vertices[0]));                     // 要素1つ分のメモリサイズ
		const int verticesSize = static_cast<int>(vertices.size() * vertexSize);          // 全体のメモリサイズ

		std::uint16_t* indicesPtr = const_cast<std::uint16_t*>(indices.data());           // 先頭ポインタ
		const int indexSize = static_cast<int>(sizeof(indices[0]));                       // 要素1つ分のメモリサイズ
		const int indicesSize = static_cast<int>(indices.size() * indexSize);             // 全体のメモリサイズ

		GraphicsBuffer vertexBuffer = GraphicsBuffer();
		GraphicsBuffer indexBuffer = GraphicsBuffer();

		if (!(vertexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, verticesSize, true)))
			RETURN_FALSE(L"頂点バッファーの作成に失敗しました");
		if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer(vertexBuffer, static_cast<void*>(verticesPtr), verticesSize))
			RETURN_FALSE(L"頂点バッファーを GPU 側にコピーすることに失敗しました");
		vertexBufferView = D3D12Helper::CreateVertexBufferView(vertexBuffer, verticesSize, vertexSize);

		if (!(indexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, indicesSize, true)))
			RETURN_FALSE(L"インデックスバッファーの作成に失敗しました");
		if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer(indexBuffer, static_cast<void*>(indicesPtr), indicesSize))
			RETURN_FALSE(L"インデックスバッファーを GPU 側にコピーすることに失敗しました");
		indexBufferView = D3D12Helper::CreateIndexBufferView(indexBuffer, indicesSize, Format::R_U16);

		RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
	}
}
