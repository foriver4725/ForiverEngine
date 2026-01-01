#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

namespace ForiverEngine
{
	class D3D12BasicFlow final
	{
	public:
		DELETE_DEFAULT_METHODS(D3D12BasicFlow);

		// 途中で処理が失敗しても、エラーを出すだけでそのまま続行する

		inline static const std::string ShaderEntryFuncVS = "VSMain";
		inline static const std::string ShaderEntryFuncPS = "PSMain";

		/// <summary>
		/// DirectX12 の基本的なオブジェクト群を一括で作成する
		/// </summary>
		static std::tuple<Factory, Device, CommandAllocator, CommandList, CommandQueue>
			CreateStandardObjects()
		{
			const Factory factory = D3D12Helper::CreateFactory();
			if (!factory)
				ShowError(L"Factory の作成に失敗しました");

			const Device device = D3D12Helper::CreateDevice(factory);
			if (!device)
				ShowError(L"Device の作成に失敗しました");

			const CommandAllocator commandAllocater = D3D12Helper::CreateCommandAllocator(device);
			if (!commandAllocater)
				ShowError(L"CommandAllocater の作成に失敗しました");

			const CommandList commandList = D3D12Helper::CreateCommandList(device, commandAllocater);
			if (!commandList)
				ShowError(L"CommandList の作成に失敗しました");

			const CommandQueue commandQueue = D3D12Helper::CreateCommandQueue(device);
			if (!commandQueue)
				ShowError(L"CommandQueue の作成に失敗しました");

			return { factory, device, commandAllocater, commandList, commandQueue };
		}

		/// <summary>
		/// 頂点バッファビューとインデックスバッファビューを一括で作成する (ブロック)
		/// </summary>
		static std::tuple<VertexBufferView, IndexBufferView>
			CreateVertexAndIndexBufferViews(const Device& device, const Mesh& mesh)
		{
			const std::vector<VertexData>& vertices = mesh.vertices;                 // メッシュのプロパティ
			const VertexData* verticesPtr = vertices.data();                         // 先頭ポインタ
			const int vertexSize = static_cast<int>(sizeof(vertices[0]));            // 要素1つ分のメモリサイズ
			const int verticesSize = static_cast<int>(vertices.size() * vertexSize); // 全体のメモリサイズ

			const std::vector<std::uint32_t>& indices = mesh.indices;                // メッシュのプロパティ
			const std::uint32_t* indicesPtr = indices.data();                        // 先頭ポインタ
			const int indexSize = static_cast<int>(sizeof(indices[0]));              // 要素1つ分のメモリサイズ
			const int indicesSize = static_cast<int>(indices.size() * indexSize);    // 全体のメモリサイズ

			const GraphicsBuffer vertexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, verticesSize, true);
			if (!vertexBuffer)
				ShowError(L"頂点バッファーの作成に失敗しました");
			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(vertexBuffer, static_cast<const void*>(verticesPtr), verticesSize))
				ShowError(L"頂点バッファーを GPU 側にコピーすることに失敗しました");
			const VertexBufferView vertexBufferView = D3D12Helper::CreateVertexBufferView(vertexBuffer, verticesSize, vertexSize);

			const GraphicsBuffer indexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, indicesSize, true);
			if (!indexBuffer)
				ShowError(L"インデックスバッファーの作成に失敗しました");
			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(indexBuffer, static_cast<const void*>(indicesPtr), indicesSize))
				ShowError(L"インデックスバッファーを GPU 側にコピーすることに失敗しました");
			const IndexBufferView indexBufferView = D3D12Helper::CreateIndexBufferView(indexBuffer, indicesSize, Format::R_U32);

			return { vertexBufferView, indexBufferView };
		}

		/// <summary>
		/// 頂点バッファビューとインデックスバッファビューを一括で作成する (板ポリ)
		/// </summary>
		static std::tuple<VertexBufferView, IndexBufferView>
			CreateVertexAndIndexBufferViews(const Device& device, const MeshQuad& mesh)
		{
			const std::vector<VertexDataQuad>& vertices = mesh.vertices;             // メッシュのプロパティ
			const VertexDataQuad* verticesPtr = vertices.data();                     // 先頭ポインタ
			const int vertexSize = static_cast<int>(sizeof(vertices[0]));            // 要素1つ分のメモリサイズ
			const int verticesSize = static_cast<int>(vertices.size() * vertexSize); // 全体のメモリサイズ

			const std::vector<std::uint32_t>& indices = mesh.indices;                // メッシュのプロパティ
			const std::uint32_t* indicesPtr = indices.data();                        // 先頭ポインタ
			const int indexSize = static_cast<int>(sizeof(indices[0]));              // 要素1つ分のメモリサイズ
			const int indicesSize = static_cast<int>(indices.size() * indexSize);    // 全体のメモリサイズ

			const GraphicsBuffer vertexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, verticesSize, true);
			if (!vertexBuffer)
				ShowError(L"頂点バッファーの作成に失敗しました");
			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(vertexBuffer, static_cast<const void*>(verticesPtr), verticesSize))
				ShowError(L"頂点バッファーを GPU 側にコピーすることに失敗しました");
			const VertexBufferView vertexBufferView = D3D12Helper::CreateVertexBufferView(vertexBuffer, verticesSize, vertexSize);

			const GraphicsBuffer indexBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, indicesSize, true);
			if (!indexBuffer)
				ShowError(L"インデックスバッファーの作成に失敗しました");
			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(indexBuffer, static_cast<const void*>(indicesPtr), indicesSize))
				ShowError(L"インデックスバッファーを GPU 側にコピーすることに失敗しました");
			const IndexBufferView indexBufferView = D3D12Helper::CreateIndexBufferView(indexBuffer, indicesSize, Format::R_U32);

			return { vertexBufferView, indexBufferView };
		}

		/// <summary>
		/// シェーダーをロードして、頂点シェーダーとピクセルシェーダーにコンパイルする
		/// </summary>
		static std::tuple<Blob, Blob>
			CompileShader_VS_PS(const std::string& path)
		{
			Blob vs = Blob();
			Blob ps = Blob();

			std::wstring errorMessage = L"";

			if (!D3D12Helper::CompileShaderFile_VS_PS(
				StringUtils::UTF8ToUTF16(path),
				ShaderEntryFuncVS, ShaderEntryFuncPS,
				vs, ps,
				errorMessage
			))
				ShowError(errorMessage.c_str());

			return { vs, ps };
		}

		/// <summary>
		/// RootSignature と GraphicsPipelineState を一括で作成して返す
		/// </summary>
		static std::tuple<RootSignature, PipelineState>
			CreateRootSignatureAndGraphicsPipelineState
			(
				const Device& device,
				const RootParameter& rootParameter,
				const SamplerConfig& samplerConfig,
				const Blob& shaderVS,
				const Blob& shaderPS,
				const std::vector<VertexLayout>& vertexLayouts,
				FillMode fillMode,
				CullMode cullMode,
				bool useDSV
			)
		{
			std::wstring errorMessage = L"";

			const RootSignature rootSignature = D3D12Helper::CreateRootSignature(device, rootParameter, samplerConfig, errorMessage);
			if (!rootSignature)
				ShowError(errorMessage.c_str());

			const PipelineState graphicsPipelineState = D3D12Helper::CreateGraphicsPipelineState(
				device, rootSignature, shaderVS, shaderPS, vertexLayouts, fillMode, cullMode, useDSV);
			if (!graphicsPipelineState)
				ShowError(L"GraphicsPipelineState の作成に失敗しました");

			return { rootSignature, graphicsPipelineState };
		}

		/// <summary>
		/// <para>GraphicsBuffer を作成して、任意の値を GPU にアップロードする</para>
		/// <para>outBufferVirtualPtr にポインタを渡すと、処理終了時にアンマップしない</para>
		/// <para>従って、outBufferVirtualPtr に対して CPU 側で変更した値が、動的に CB に反映されるようになる</para>
		/// <para>作成したバッファを返す</para>
		/// <para>CBV を作る用のバッファ</para>
		/// </summary>
		template<typename TCBData>
		static GraphicsBuffer
			InitCBVBuffer(const Device& device, const TCBData& data, TCBData** outBufferVirtualPtr = nullptr)
		{
			if (outBufferVirtualPtr)
				*outBufferVirtualPtr = nullptr;

			// 256 アラインメントにする必要がある
			const GraphicsBuffer buffer = D3D12Helper::CreateGraphicsBuffer1D(device, GetAlignmentedSize(sizeof(TCBData), 256), true);
			if (!buffer)
				ShowError(L"定数バッファーの作成に失敗しました");

			if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(
				buffer, &data, sizeof(TCBData), !outBufferVirtualPtr, reinterpret_cast<void**>(outBufferVirtualPtr)))
				ShowError(L"定数バッファーへのデータ転送に失敗しました");

			return buffer;
		}

		/// <summary>
		/// <para>GraphicsBuffer を作成して、テクスチャデータを GPU にアップロードする</para>
		/// <para>テクスチャは与えられたパスからロードし、パスが1つなら2Dテクスチャとして、複数あるなら2Dテクスチャ配列として処理される</para>
		/// <para>作成したバッファと、ロードしたテクスチャのメタデータを返す</para>
		/// <para>SRV を作る用のバッファ</para>
		/// </summary>
		static std::tuple<GraphicsBuffer, Texture>
			InitSRVBuffer
			(
				const Device& device,
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const CommandAllocator& commandAllocator,
				const std::vector<std::string>& paths
			)
		{
			Texture texture = Texture();

			if (paths.empty())
				ShowError(L"テクスチャ(群)のパスが空です");
			if (paths.size() <= 1)
				texture = D3D12Helper::LoadAsTexture(paths[0]);
			else
				texture = D3D12Helper::LoadAsTextureArray(paths);
			if (!texture.IsValid())
				ShowError(L"テクスチャ(群)のロードに失敗しました");

			const GraphicsBuffer textureBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, texture,
				GraphicsBufferUsagePermission::None, GraphicsBufferState::CopyDestination, Color::Transparent());
			if (!textureBuffer)
				ShowError(L"テクスチャ(配列)バッファの作成に失敗しました");

			D3D12BasicFlow::UploadTextureToGPU(commandList, commandQueue, commandAllocator, device, textureBuffer, texture);

			return { textureBuffer, texture };
		}

		/// <summary>
		/// <para>GraphicsBuffer を作成して、テクスチャデータを GPU にアップロードする</para>
		/// <para>テクスチャを手動で与える (2Dテクスチャ or 2Dテクスチャ配列)</para>
		/// <para>テクスチャの種類が 2D でない場合は、失敗させる</para>
		/// <para>作成したバッファを返す</para>
		/// <para>SRV を作る用のバッファ</para>
		/// </summary>
		static GraphicsBuffer
			InitSRVBuffer
			(
				const Device& device,
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const CommandAllocator& commandAllocator,
				const Texture& texture
			)
		{
			const GraphicsBuffer textureBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, texture,
				GraphicsBufferUsagePermission::None, GraphicsBufferState::CopyDestination, Color::Transparent());
			if (!textureBuffer)
				ShowError(L"テクスチャ(配列)バッファの作成に失敗しました");

			D3D12BasicFlow::UploadTextureToGPU(commandList, commandQueue, commandAllocator, device, textureBuffer, texture);

			return textureBuffer;
		}

		/// <summary>
		/// <para>- CBV/SRV/UAV 用の DescriptorHeap を作成する (UAV は設定できない)</para>
		/// <para>- CBV,SRV の順、さらに配列の前のものから順に、バッファからビューを作成して DescriptorHeap に登録する</para>
		/// <para>- 作成した DescriptorHeap を返す</para>
		/// <para>※ バッファの個数は、ルートパラメーターで登録したものと同じにすること</para>
		/// <para>※ シェーダーからは見える</para>
		/// </summary>
		static DescriptorHeap
			InitDescriptorHeapBasic
			(
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
				ShowError(L"DescriptorHeap (CBV_SRV_UAV) の作成に失敗しました");

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

			return descriptorHeapBasic;
		}

		/// <summary>
		/// <para>SwapChain 内の RT 全てに対して、専用の DescriptorHeap を作成し、RTV をその DescriptorHeap の中に作成して返す</para>
		/// <para>戻り値は関数で、インデックスを基に、RT/RTV を取得出来る</para>
		/// </summary>
		static std::tuple<std::function<GraphicsBuffer(int)>, std::function<DescriptorHeapHandleAtCPU(int)>>
			InitRTV(const Device& device, const SwapChain& swapChain, Format format)
		{
			const int descriptorCount = D3D12Helper::GetRTCount(swapChain);
			if (descriptorCount <= 0)
				ShowError(L"SwapChain 内の RT の数が不正です");

			const DescriptorHeap descriptorHeapRTV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::RTV, descriptorCount, false);
			if (!descriptorHeapRTV)
				ShowError(L"DescriptorHeap (RTV) の作成に失敗しました");

			if (!D3D12Helper::CreateRenderTargetViews(device, descriptorHeapRTV, swapChain, format))
				ShowError(L"RenderTargetView を作成できない RenderTargetBuffer がありました");

			const std::function<GraphicsBuffer(int)> bufferGetter = [swapChain](int i) { return D3D12Helper::GetRT(swapChain, i); };
			const std::function<DescriptorHeapHandleAtCPU(int)> viewGetter = [device, descriptorHeapRTV](int i) {
				return D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
					device, descriptorHeapRTV, DescriptorHeapType::RTV, i);
				};

			return { bufferGetter, viewGetter };
		}

		/// <summary>
		/// <para>与えられた RT に対して、専用の DescriptorHeap を作成し、RTV をその DescriptorHeap の中に作成して返す</para>
		/// <para>作成した RTV を返す</para>
		/// </summary>
		static DescriptorHeapHandleAtCPU
			InitRTV(const Device& device, const GraphicsBuffer& rt, Format format)
		{
			const DescriptorHeap descriptorHeapRTV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::RTV, 1, false);
			if (!descriptorHeapRTV)
				ShowError(L"DescriptorHeap (RTV) の作成に失敗しました");

			D3D12Helper::CreateRenderTargetView(device, descriptorHeapRTV, rt, format, 0);

			const DescriptorHeapHandleAtCPU rtv = D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
				device, descriptorHeapRTV, DescriptorHeapType::RTV, 0
			);

			return rtv;
		}

		/// <summary>
		/// <para>深度バッファと専用の DescriptorHeap を作成し、DSV をその DescriptorHeap の中に作成して返す</para>
		/// <para>記録用のバッファなので、1つのみ作成する</para>
		/// <para>ステンシルは使わないので、深度のみとして作成する</para>
		/// </summary>
		static DescriptorHeapHandleAtCPU
			InitDSV(const Device& device, const Lattice2& size)
		{
			const Texture depthBufferMetadata = Texture::CreateManually({}, size, Format::D_F32);
			const GraphicsBuffer depthBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, depthBufferMetadata,
				GraphicsBufferUsagePermission::AllowDepthStencil, GraphicsBufferState::DepthWrite, Color(DepthBufferClearValue, 0, 0, 0));
			if (!depthBuffer)
				ShowError(L"DepthBuffer の作成に失敗しました");

			const DescriptorHeap descriptorHeapDSV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::DSV, 1, false);
			if (!descriptorHeapDSV)
				ShowError(L"DescriptorHeap (DSV) の作成に失敗しました");

			D3D12Helper::CreateDepthStencilView(device, descriptorHeapDSV, depthBuffer);

			const DescriptorHeapHandleAtCPU dsv = D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
				device, descriptorHeapDSV, DescriptorHeapType::DSV, 0);

			return dsv;
		}

		/// <summary>
		/// <para>[Command]</para>
		/// <para>コマンドリストをクローズして実行し、GPUの処理が完了するまで待機する</para>
		/// </summary>
		static void
			CommandCloseAndWaitForCompletion(const CommandList& commandList, const CommandQueue& commandQueue, const Device& device)
		{
			D3D12Helper::CommandClose(commandList);

			D3D12Helper::ExecuteCommands(commandQueue, commandList);

			if (!D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue))
				ShowError(L"GPU の処理待ち受けに失敗しました");
		}

		/// <summary>
		/// <para>テクスチャデータを GPU 側にアップロードする</para>
		/// <para>内部で中間バッファを作成し、転送する</para>
		/// </summary>
		static void
			UploadTextureToGPU
			(
				const CommandList& commandList,
				const CommandQueue& commandQueue,
				const CommandAllocator& commandAllocator,
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
				ShowError(L"テクスチャ転送用中間バッファの作成に失敗しました");

			if (!D3D12Helper::CommandCopyDataFromCPUToGPUThroughGraphicsBufferTexture2D(
				commandList, intermediateBuffer, textureBuffer, textureAsMetadata))
				ShowError(L"テクスチャデータを GPU 側にコピーするコマンドの発行に失敗しました");

			D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, textureBuffer,
				GraphicsBufferState::CopyDestination, GraphicsBufferState::PixelShaderResource, false);

			D3D12BasicFlow::CommandCloseAndWaitForCompletion(commandList, commandQueue, device);
			// コマンドを実行し終わってから、クリアする
			if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocator, commandList))
				ShowError(L"CommandAllocator, CommandList のクリアに失敗しました");
		}

		/// <summary>
		/// <para>[Command]</para>
		/// <para>ループ内の、基本的なコマンド系処理を実行する</para>
		/// <para>配列の要素数はドローコール数と同じ!</para>
		/// </summary>
		/// <param name="commandList">CommandList</param>
		/// <param name="commandQueue">CommandQueue</param>
		/// <param name="commandAllocator">CommandAllocator</param>
		/// <param name="device">Device</param>
		/// <param name="rootSignature">RootSignature</param>
		/// <param name="graphicsPipelineState">Graphics PipelineState</param>
		/// <param name="rt">RT</param>
		/// <param name="rtv">RTV</param>
		/// <param name="dsv">DSV (nullptr ならば、使わないものとみなす)</param>
		/// <param name="descriptorHeapBasics">CBV/SRV/UAV 用 DescriptorHeap (0番目のルートパラメーターに紐づける想定なので、1つしか渡せない)</param>
		/// <param name="vertexBufferViewArray">頂点バッファビュー (サイズはドローコール数と同じ!)</param>
		/// <param name="indexBufferViewArray">インデックスバッファビュー (サイズはドローコール数と同じ!)</param>
		/// <param name="rtStateOutsideRender">レンダー外の RT の状態</param>
		/// <param name="rtStateInsideRender">レンダー内の RT の状態</param>
		/// <param name="viewportScissorRect">ビューポートとシザー矩形</param>
		/// <param name="primitiveTopology">プリミティブのトポロジー</param>
		/// <param name="rtvClearColor">RTV のクリアカラー</param>
		/// <param name="depthClearValue">DSV のクリア深度値 (ステンシルは使わないので、深度値のみ. [0, 1])</param>
		/// <param name="indexTotalCountArray">ドローコール時のインデックス総数 (サイズはドローコール数と同じ!)</param>
		static void
			CommandBasicLoop
			(
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
				GraphicsBufferState rtStateOutsideRender, GraphicsBufferState rtStateInsideRender,
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
				ShowError(L"頂点バッファビューの数と、ドローコール数が一致しません");
			if (drawCount != static_cast<std::uint32_t>(indexBufferViewArray.size()))
				ShowError(L"インデックスバッファビューの数と、ドローコール数が一致しません");

			D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, rt, rtStateOutsideRender, rtStateInsideRender, false);
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
			D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, rt, rtStateInsideRender, rtStateOutsideRender, false);

			D3D12BasicFlow::CommandCloseAndWaitForCompletion(commandList, commandQueue, device);
			// コマンドを実行し終わってから、クリアする
			if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocator, commandList))
				ShowError(L"CommandAllocator, CommandList のクリアに失敗しました");
		}

		/// <summary>
		/// MVP行列を計算して返す
		/// </summary>
		static Matrix4x4
			CalculateMVPMatrix(const Transform& transform, const CameraTransform& cameraTransform)
		{
			const Matrix4x4 m = transform.CalculateModelMatrix();
			const Matrix4x4 vp = cameraTransform.CalculateVPMatrix();

			return vp * m;
		}
	};
}
