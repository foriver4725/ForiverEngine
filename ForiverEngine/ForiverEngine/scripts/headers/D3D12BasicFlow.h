#pragma once

#include <scripts/common/Include.h>

#include "./WindowHelper.h"
#include "./D3D12Helper.h"
#include "./AssetLoader.h"

namespace ForiverEngine
{
	inline static const std::string ShaderEntryFuncVS = "VSMain";
	inline static const std::string ShaderEntryFuncPS = "PSMain";

	class D3D12BasicFlow final
	{
	public:
		DELETE_DEFAULT_METHODS(D3D12BasicFlow);

#pragma region With error check (useful)

		static void Check(std::tuple<bool, std::wstring>&& value)
		{
			auto& [success, errorMessage] = value;

			if (!success)
				ShowError(errorMessage.c_str());
		}

		template<typename T>
		static T Check(std::tuple<bool, std::wstring, std::tuple<T>>&& value)
		{
			auto& [success, errorMessage, output] = value;

			if (!success)
				ShowError(errorMessage.c_str());

			return std::get<0>(output);
		}

		template<typename... Types>
		static std::tuple<Types...> Check(std::tuple<bool, std::wstring, std::tuple<Types...>>&& value)
		{
			auto& [success, errorMessage, output] = value;

			if (!success)
				ShowError(errorMessage.c_str());

			return output;
		}

		/// <summary>
		/// DirectX12 の基本的なオブジェクト群を一括で作成する
		/// </summary>
		static std::tuple<Factory, Device, CommandAllocator, CommandList, CommandQueue, SwapChain>
			CreateStandardObjects(
				HWND hwnd,
				int windowWidth,
				int windowHeight
			)
		{
			return Check(CreateStandardObjects_Impl(hwnd, windowWidth, windowHeight));
		}

		/// <summary>
		/// 頂点バッファビューとインデックスバッファビューを一括で作成する
		/// </summary>
		static std::tuple<VertexBufferView, IndexBufferView>
			CreateVertexAndIndexBufferViews(
				const Device& device,
				const Mesh& mesh
			)
		{
			return Check(CreateVertexAndIndexBufferViews_Impl(device, mesh));
		}

		/// <summary>
		/// シェーダーをロードして、頂点シェーダーとピクセルシェーダーにコンパイルする
		/// </summary>
		static std::tuple<Blob, Blob>
			CompileShader_VS_PS(
				const std::string& path
			)
		{
			return Check(CompileShader_VS_PS_Impl(path));
		}

		/// <summary>
		/// RootSignature と GraphicsPipelineState を一括で作成して返す
		/// </summary>
		static std::tuple<RootSignature, PipelineState>
			CreateRootSignatureAndGraphicsPipelineState(
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
			return Check(CreateRootSignatureAndGraphicsPipelineState_Impl(
				device, rootParameter, samplerConfig, shaderVS, shaderPS, vertexLayouts, fillMode, cullMode));
		}

		/// <summary>
		/// <para>GraphicsBuffer を作成して、任意の値を GPU にアップロードする</para>
		/// <para>作成したバッファを返す</para>
		/// <para>CBV を作る用のバッファ</para>
		/// </summary>
		template<typename T>
		static GraphicsBuffer
			InitCBVBuffer(
				const Device& device,
				const T& data,
				bool unmapOnEnd = true,
				T** outBufferVirtualPtr = nullptr
			)
		{
			return Check(InitCBVBuffer_Impl(device, data, unmapOnEnd, outBufferVirtualPtr));
		}

		/// <summary>
		/// <para>GraphicsBuffer を作成して、テクスチャデータを GPU にアップロードする</para>
		/// <para>テクスチャは与えられたパスからロードし、パスが複数あるならテクスチャ配列として処理される</para>
		/// <para>作成したバッファと、ロードしたテクスチャのメタデータを返す</para>
		/// <para>SRV を作る用のバッファ</para>
		/// </summary>
		static std::tuple<GraphicsBuffer, Texture>
			InitSRVBuffer(
				const Device& device,
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const std::vector<std::string>& paths
			)
		{
			return Check(InitSRVBuffer_Impl(device, commandList, commandQueue, paths));
		}

		/// <summary>
		/// <para>- CBV/SRV/UAV 用の DescriptorHeap を作成する (UAV は設定できない)</para>
		/// <para>- CBV,SRV の順、さらに配列の前のものから順に、バッファからビューを作成して DescriptorHeap に登録する</para>
		/// <para>- 作成した DescriptorHeap を返す</para>
		/// <para>※ バッファの個数は、ルートパラメーターで登録したものと同じにすること</para>
		/// <para>※ シェーダーからは見える</para>
		/// </summary>
		static DescriptorHeap
			InitDescriptorHeapBasic(
				const Device& device,
				const std::vector<GraphicsBuffer>& cbvBuffers,
				const std::vector<std::tuple<GraphicsBuffer, Texture>>& srvBuffers
			)
		{
			return Check(InitDescriptorHeapBasic_Impl(device, cbvBuffers, srvBuffers));
		}

		/// <summary>
		/// <para>専用の DescriptorHeap を作成し、複数個の RTV をその DescriptorHeap の中に作成して返す</para>
		/// <para>戻り値は関数で、インデックスを基に、バッファとビューを取得出来る</para>
		/// </summary>
		static std::tuple<std::function<GraphicsBuffer(int)>, std::function<DescriptorHeapHandleAtCPU(int)>>
			InitRTV(
				const Device& device,
				const SwapChain& swapChain,
				int amount,
				bool sRGB
			)
		{
			return Check(InitRTV_Impl(device, swapChain, amount, sRGB));
		}

		/// <summary>
		/// <para>深度バッファと専用の DescriptorHeap を作成し、DSV をその DescriptorHeap の中に作成して返す</para>
		/// <para>記録用のバッファなので、1つのみ作成する</para>
		/// <para>ステンシルは使わないので、深度のみとして作成する</para>
		/// </summary>
		static DescriptorHeapHandleAtCPU
			InitDSV(
				const Device& device,
				int width,
				int height,
				float depthClearValue
			)
		{
			return Check(InitDSV_Impl(device, width, height, depthClearValue));
		}

		/// <summary>
		/// <para>[Command]</para>
		/// <para>コマンドリストをクローズして実行し、GPUの処理が完了するまで待機する</para>
		/// </summary>
		static void
			CommandCloseAndWaitForCompletion(
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const Device& device
			)
		{
			Check(CommandCloseAndWaitForCompletion_Impl(commandList, commandQueue, device));
		}

		/// <summary>
		/// <para>テクスチャデータを GPU 側にアップロードする</para>
		/// <para>内部で中間バッファを作成し、転送する</para>
		/// </summary>
		static void
			UploadTextureToGPU(
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const Device& device,
				const GraphicsBuffer& textureBuffer,
				const Texture& textureAsMetadata
			)
		{
			Check(UploadTextureToGPU_Impl(
				commandList, commandQueue, device, textureBuffer, textureAsMetadata));
		}

		/// <summary>
		/// <para>[Command]</para>
		/// <para>ループ内の、基本的なコマンド系処理を実行する</para>
		/// </summary>
		/// <param name="commandList">CommandList</param>
		/// <param name="commandQueue">CommandQueue</param>
		/// <param name="commandAllocator">CommandAllocator</param>
		/// <param name="device">Device</param>
		/// <param name="rootSignature">RootSignature</param>
		/// <param name="graphicsPipelineState">Graphics PipelineState</param>
		/// <param name="currentBackBuffer">現在のバックバッファ</param>
		/// <param name="currentBackBufferRTV">現在のバックバッファの RTV</param>
		/// <param name="dsv">DSV</param>
		/// <param name="descriptorHeapBasics">CBV/SRV/UAV 用 DescriptorHeap (0番目のルートパラメーターに紐づける想定なので、1つしか渡せない)</param>
		/// <param name="vertexBufferViews">頂点バッファビュー群</param>
		/// <param name="indexBufferView">インデックスバッファビュー</param>
		/// <param name="viewportScissorRect">ビューポートとシザー矩形</param>
		/// <param name="primitiveTopology">プリミティブのトポロジー</param>
		/// <param name="rtvClearColor">RTV のクリアカラー</param>
		/// <param name="depthClearValue">DSV のクリア深度値 (ステンシルは使わないので、深度値のみ. [0, 1])</param>
		/// <param name="indexTotalCount">ドローコール時のインデックス総数</param>
		static void
			CommandBasicLoop(
				// 基本オブジェクト
				const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
				const Device& device,
				// パイプライン関連
				const RootSignature& rootSignature, const PipelineState& graphicsPipelineState, const GraphicsBuffer& currentBackBuffer,
				// Descriptor
				const DescriptorHeapHandleAtCPU& currentBackBufferRTV, const DescriptorHeapHandleAtCPU& dsv,
				const DescriptorHeap& descriptorHeapBasic,
				const std::vector<VertexBufferView>& vertexBufferViews, const IndexBufferView& indexBufferView,
				// 数値情報
				const ViewportScissorRect& viewportScissorRect, PrimitiveTopology primitiveTopology,
				Color rtvClearColor, float depthClearValue,
				// ドローコール関連
				int indexTotalCount
			)
		{
			Check(CommandBasicLoop_Impl(
				commandList, commandQueue, commandAllocator, device,
				rootSignature, graphicsPipelineState, currentBackBuffer,
				currentBackBufferRTV, dsv, descriptorHeapBasic, vertexBufferViews, indexBufferView,
				viewportScissorRect, primitiveTopology, rtvClearColor, depthClearValue, indexTotalCount));
		}

		/// <summary>
		/// MVP行列を計算して返す
		/// </summary>
		static Matrix4x4
			CalculateMVPMatrix(
				const Transform& transform,
				const CameraTransform& cameraTransform
			)
		{
			return Check(CalculateMVPMatrix_Impl(transform, cameraTransform));
		}

#pragma endregion

#pragma region Implementation

		// 戻り値の構造は共通化する
		// 1番目 : 成功したら true, 失敗したら false
		// 2番目 : エラーメッセージ (成功したら空文字列)
		// 失敗した段階で処理を中断する

		/// <summary>
		/// DirectX12 の基本的なオブジェクト群を一括で作成する
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<Factory, Device, CommandAllocator, CommandList, CommandQueue, SwapChain>>
			CreateStandardObjects_Impl(
				HWND hwnd,
				int windowWidth,
				int windowHeight
			);

		/// <summary>
		/// 頂点バッファビューとインデックスバッファビューを一括で作成する
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<VertexBufferView, IndexBufferView>>
			CreateVertexAndIndexBufferViews_Impl(
				const Device& device,
				const Mesh& mesh
			);

		/// <summary>
		/// シェーダーをロードして、頂点シェーダーとピクセルシェーダーにコンパイルする
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<Blob, Blob>>
			CompileShader_VS_PS_Impl(
				const std::string& path
			);

		/// <summary>
		/// RootSignature と GraphicsPipelineState を一括で作成して返す
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<RootSignature, PipelineState>>
			CreateRootSignatureAndGraphicsPipelineState_Impl(
				const Device& device,
				const RootParameter& rootParameter,
				const SamplerConfig& samplerConfig,
				const Blob& shaderVS,
				const Blob& shaderPS,
				const std::vector<VertexLayout>& vertexLayouts,
				FillMode fillMode,
				CullMode cullMode
			);

		/// <summary>
		/// <para>GraphicsBuffer を作成して、任意の値を GPU にアップロードする</para>
		/// <para>作成したバッファを返す</para>
		/// <para>CBV を作る用のバッファ</para>
		/// </summary>
		template<typename T>
		static std::tuple<bool, std::wstring, std::tuple<GraphicsBuffer>>
			InitCBVBuffer_Impl(
				const Device& device,
				const T& data,
				bool unmapOnEnd = true,
				T** outBufferVirtualPtr = nullptr
			)
		{
			if (outBufferVirtualPtr)
				*outBufferVirtualPtr = nullptr;

			GraphicsBuffer buffer = GraphicsBuffer();

#define RETURN_FALSE(errorMessage) \
			return { false, errorMessage, { buffer } };
#define RETURN_TRUE() \
			return { true, L"", { buffer } };

			// 256 アラインメントにする必要がある
			buffer = D3D12Helper::CreateGraphicsBuffer1D(device, GetAlignmentedSize(sizeof(T), 256), true);
			if (!buffer)
				RETURN_FALSE(L"定数バッファーの作成に失敗しました");

			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(
				buffer, &data, sizeof(T), unmapOnEnd, reinterpret_cast<void**>(outBufferVirtualPtr)))
				RETURN_FALSE(L"定数バッファーへのデータ転送に失敗しました");

			RETURN_TRUE();

#undef RETURN_FALSE
#undef RETURN_TRUE
		}

		/// <summary>
		/// <para>GraphicsBuffer を作成して、テクスチャデータを GPU にアップロードする</para>
		/// <para>テクスチャは与えられたパスからロードし、パスが複数あるならテクスチャ配列として処理される</para>
		/// <para>作成したバッファと、ロードしたテクスチャのメタデータを返す</para>
		/// <para>SRV を作る用のバッファ</para>
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<GraphicsBuffer, Texture>>
			InitSRVBuffer_Impl(
				const Device& device,
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const std::vector<std::string>& paths
			);

		/// <summary>
		/// <para>- CBV/SRV/UAV 用の DescriptorHeap を作成する (UAV は設定できない)</para>
		/// <para>- CBV,SRV の順、さらに配列の前のものから順に、バッファからビューを作成して DescriptorHeap に登録する</para>
		/// <para>- 作成した DescriptorHeap を返す</para>
		/// <para>※ バッファの個数は、ルートパラメーターで登録したものと同じにすること</para>
		/// <para>※ シェーダーからは見える</para>
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<DescriptorHeap>>
			InitDescriptorHeapBasic_Impl(
				const Device& device,
				const std::vector<GraphicsBuffer>& cbvBuffers,
				const std::vector<std::tuple<GraphicsBuffer, Texture>>& srvBuffers
			);

		/// <summary>
		/// <para>専用の DescriptorHeap を作成し、複数個の RTV をその DescriptorHeap の中に作成して返す</para>
		/// <para>戻り値は関数で、インデックスを基に、バッファとビューを取得出来る</para>
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<std::function<GraphicsBuffer(int)>, std::function<DescriptorHeapHandleAtCPU(int)>>>
			InitRTV_Impl(
				const Device& device,
				const SwapChain& swapChain,
				int amount,
				bool sRGB
			);

		/// <summary>
		/// <para>深度バッファと専用の DescriptorHeap を作成し、DSV をその DescriptorHeap の中に作成して返す</para>
		/// <para>記録用のバッファなので、1つのみ作成する</para>
		/// <para>ステンシルは使わないので、深度のみとして作成する</para>
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<DescriptorHeapHandleAtCPU>>
			InitDSV_Impl(
				const Device& device,
				int width,
				int height,
				float depthClearValue
			);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>コマンドリストをクローズして実行し、GPUの処理が完了するまで待機する</para>
		/// </summary>
		static std::tuple<bool, std::wstring>
			CommandCloseAndWaitForCompletion_Impl(
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const Device& device
			);

		/// <summary>
		/// <para>テクスチャデータを GPU 側にアップロードする</para>
		/// <para>内部で中間バッファを作成し、転送する</para>
		/// </summary>
		static std::tuple<bool, std::wstring>
			UploadTextureToGPU_Impl(
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const Device& device,
				const GraphicsBuffer& textureBuffer,
				const Texture& textureAsMetadata
			);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>ループ内の、基本的なコマンド系処理を実行する</para>
		/// </summary>
		/// <param name="commandList">CommandList</param>
		/// <param name="commandQueue">CommandQueue</param>
		/// <param name="commandAllocator">CommandAllocator</param>
		/// <param name="device">Device</param>
		/// <param name="rootSignature">RootSignature</param>
		/// <param name="graphicsPipelineState">Graphics PipelineState</param>
		/// <param name="currentBackBuffer">現在のバックバッファ</param>
		/// <param name="currentBackBufferRTV">現在のバックバッファの RTV</param>
		/// <param name="dsv">DSV</param>
		/// <param name="descriptorHeapBasics">CBV/SRV/UAV 用 DescriptorHeap (0番目のルートパラメーターに紐づける想定なので、1つしか渡せない)</param>
		/// <param name="vertexBufferViews">頂点バッファビュー群</param>
		/// <param name="indexBufferView">インデックスバッファビュー</param>
		/// <param name="viewportScissorRect">ビューポートとシザー矩形</param>
		/// <param name="primitiveTopology">プリミティブのトポロジー</param>
		/// <param name="rtvClearColor">RTV のクリアカラー</param>
		/// <param name="depthClearValue">DSV のクリア深度値 (ステンシルは使わないので、深度値のみ. [0, 1])</param>
		/// <param name="indexTotalCount">ドローコール時のインデックス総数</param>
		static std::tuple<bool, std::wstring>
			CommandBasicLoop_Impl(
				// 基本オブジェクト
				const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
				const Device& device,
				// パイプライン関連
				const RootSignature& rootSignature, const PipelineState& graphicsPipelineState, const GraphicsBuffer& currentBackBuffer,
				// Descriptor
				const DescriptorHeapHandleAtCPU& currentBackBufferRTV, const DescriptorHeapHandleAtCPU& dsv,
				const DescriptorHeap& descriptorHeapBasic,
				const std::vector<VertexBufferView>& vertexBufferViews, const IndexBufferView& indexBufferView,
				// 数値情報
				const ViewportScissorRect& viewportScissorRect, PrimitiveTopology primitiveTopology,
				Color rtvClearColor, float depthClearValue,
				// ドローコール関連
				int indexTotalCount
			);

		/// <summary>
		/// MVP行列を計算して返す
		/// </summary>
		static std::tuple<bool, std::wstring, std::tuple<Matrix4x4>>
			CalculateMVPMatrix_Impl(
				const Transform& transform,
				const CameraTransform& cameraTransform
			);

#pragma endregion
	};
}
