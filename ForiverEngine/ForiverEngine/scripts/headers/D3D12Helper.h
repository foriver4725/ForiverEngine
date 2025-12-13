#pragma once

#include <scripts/common/Include.h>
#include "./D3D12Defines.h"

namespace ForiverEngine
{
	class D3D12Helper final
	{
	public:
		DELETE_DEFAULT_METHODS(D3D12Helper);

		/// <summary>
		/// DXGIFactory を作成して返す (失敗したら nullptr)
		/// </summary>
		static Factory CreateFactory();

		/// <summary>
		/// <para>D3D12Device を作成して返す (失敗したら nullptr)</para>
		/// なるべく最新の FeatureLevel に対応したものを探す
		/// </summary>
		static Device CreateDevice(const Factory& factory);

		/// <summary>
		/// RootSignature を作成して返す (失敗したら nullptr)
		/// </summary>
		static RootSignature CreateRootSignature(
			const Device& device, const RootParameter& rootParameter, const SamplerConfig& samplerConfig, std::wstring& outErrorMessage);

		/// <summary>
		/// GraphicsPipelineState を作成して返す (失敗したら nullptr)
		/// </summary>
		static PipelineState CreateGraphicsPipelineState(
			const Device& device, const RootSignature& rootSignature, const Blob& vs, const Blob& ps,
			const std::vector<VertexLayout>& vertexLayouts, FillMode fillMode, CullMode cullMode);

		/// <summary>
		/// CommandAllocator を作成して返す (失敗したら nullptr)
		/// </summary>
		static CommandAllocator CreateCommandAllocator(const Device& device);

		/// <summary>
		/// CommandList を作成して返す (失敗したら nullptr)
		/// </summary>
		static CommandList CreateCommandList(const Device& device, const CommandAllocator& commandAllocator);

		/// <summary>
		/// CommandQueue を作成して返す (失敗したら nullptr)
		/// </summary>
		static CommandQueue CreateCommandQueue(const Device& device);

		/// <summary>
		/// DXGISwapChain を作成して返す (失敗したら nullptr)
		/// </summary>
		static SwapChain CreateSwapChain(const Factory& factory, const CommandQueue& commandQueue, HWND hwnd, int windowWidth, int windowHeight);

		/// <summary>
		/// DescriptorHeap を作成して返す (失敗したら nullptr)
		/// </summary>
		static DescriptorHeap CreateDescriptorHeap(const Device& device, DescriptorHeapType type, int descriptorAmount, bool visibleToShader);

		/// <summary>
		/// Fence を作成して返す (失敗したら nullptr)
		/// </summary>
		static Fence CreateFence(const Device& device);

		/// <summary>
		/// <para>GPU側のメモリ領域を確保し、その GraphicsBuffer を返す (失敗したら nullptr)</para>
		/// 1次元配列用
		/// </summary>
		static GraphicsBuffer CreateGraphicsBuffer1D(const Device& device, int size, bool canMapFromCPU);

		/// <summary>
		/// <para>GPU側のメモリ領域を確保し、その GraphicsBuffer を返す (失敗したら nullptr)</para>
		/// <para>2次元テクスチャ用 (テクスチャのタイプが2Dで無いならば、失敗とみなし nullptr を返す)</para>
		/// <para>GPU内でのみ用いる想定で、CPUからのマップ不可</para>
		/// <para>テクスチャ変数のメタデータを基に作成する</para>
		/// </summary>
		static GraphicsBuffer CreateGraphicsBufferTexture2D(const Device& device, const Texture& texture);

		/// <summary>
		/// 頂点バッファ から 頂点バッファービュー を作成して返す
		/// </summary>
		/// <param name="verticesSize">頂点座標配列の sizeof()</param>
		/// <param name="vertexSize">頂点座標配列の 要素1つ分の sizeof()</param>
		static VertexBufferView CreateVertexBufferView(const GraphicsBuffer& vertexBuffer, int verticesSize, int vertexSize);

		/// <summary>
		/// インデックスバッファ から インデックスバッファービュー を作成して返す
		/// </summary>
		/// <param name="indicesSize">インデックス配列の sizeof()</param>
		/// <param name="indexFormat">インデックス配列の 要素1つ分の フォーマット</param>
		/// <returns></returns>
		static IndexBufferView CreateIndexBufferView(const GraphicsBuffer& indexBuffer, int indicesSize, Format indexFormat);

		/// <summary>
		/// <para>定数バッファ から 定数バッファビュー を作成し、DescriptorHeap に登録する</para>
		/// <para>DescriptorHeap の index 番目に登録する</para>
		/// 戻り値として取得することは出来ない!
		/// </summary>
		static void CreateCBVAndRegistToDescriptorHeap(
			const Device& device, const DescriptorHeap& descriptorHeap, const GraphicsBuffer& graphicsBuffer, int index);

		/// <summary>
		/// <para>テクスチャバッファ から シェーダーリソースビュー を作成し、DescriptorHeap に登録する</para>
		/// <para>DescriptorHeap の index 番目に登録する</para>
		/// 戻り値として取得することは出来ない!
		/// </summary>
		static void CreateSRVAndRegistToDescriptorHeap(
			const Device& device, const DescriptorHeap& descriptorHeap, const GraphicsBuffer& graphicsBuffer, int index, Format format);

		/// <summary>
		/// <para>DescriptorHeap のハンドルを作成し、index 番目の Descriptor を指し示すように内部ポインタを進めて返す</para>
		/// CPU 用
		/// </summary>
		static DescriptorHeapHandleAtCPU CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
			const Device& device, const DescriptorHeap& descriptorHeap, DescriptorHeapType descriptorHeapType, int index);

		/// <summary>
		/// <para>DescriptorHeap のハンドルを作成し、index 番目の Descriptor を指し示すように内部ポインタを進めて返す</para>
		/// GPU 用
		/// </summary>
		static DescriptorHeapHandleAtGPU CreateDescriptorHeapHandleAtGPUIndicatingDescriptorByIndex(
			const Device& device, const DescriptorHeap& descriptorHeap, DescriptorHeapType descriptorHeapType, int index);

		/// <summary>
		/// <para>GraphicsBuffer の Map() を使って、CPUのバッファをGPU側にコピーする</para>
		/// <para>1次元配列用</para>
		/// <para>バッファのサイズは、GraphicsBuffer 作成時に指定したサイズと同じであること! (一部のバッファのみコピー、などには未対応)</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool CopyDataFromCPUToGPUThroughGraphicsBuffer1D(
			const GraphicsBuffer& graphicsBuffer, const void* dataBegin, int dataSize);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>CPUのバッファをGPU側にコピーする</para>
		/// <para>2次元テクスチャ用 (テクスチャのタイプが2Dで無いならば、失敗とみなし false を返す)</para>
		/// <para>バッファのサイズは、GraphicsBuffer 作成時に指定したサイズと同じであること! (一部のバッファのみコピー、などには未対応)</para>
		/// <para> textureCopyIntermediateBuffer : CPU からデータをアップロードするための中間バッファ</para>
		/// <para> textureBuffer : 実際にテクスチャとして使われるバッファ</para>
		/// <para> CPU のテクスチャ生データは textureCopyIntermediateBuffer にマップされ、その後 textureBuffer にコピーされる</para>
		/// <para>成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)</para>
		/// TODO: 中で色々やり過ぎている! 後でリファクタしたい
		/// </summary>
		static bool CommandCopyDataFromCPUToGPUThroughGraphicsBufferTexture2D(
			const CommandList& commandList,
			const GraphicsBuffer& textureCopyIntermediateBuffer, const GraphicsBuffer& textureBuffer, const Texture& texture);

		/// <summary>
		/// <para>DescriptorHeap(RTV) と SwapChain を関連付ける</para>
		/// <para>Descriptorの数 = バッファの数だけ、繰り返し処理を行う</para>
		/// 全て成功したら true, 1つでも失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool LinkDescriptorHeapRTVToSwapChain(
			const Device& device, const DescriptorHeap& descriptorHeapRTV, const SwapChain& swapChain, bool sRGB);

		/// <summary>
		/// <para>CommandAllocator と CommandList をクリアする</para>
		/// 全て成功したら true, 1つでも失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool ClearCommandAllocatorAndList(const CommandAllocator& commandAllocator, const CommandList& commandList);

		/// <summary>
		/// <para>SwapChain から現在のバックバッファのインデックスを取得する</para>
		/// 常に、必ず1つのバックバッファが存在する想定
		/// </summary>
		static int GetCurrentBackBufferIndex(const SwapChain& swapChain);

		/// <summary>
		/// SwapChain から指定インデックスのバッファを取得する (失敗したら nullptr)
		/// </summary>
		static GraphicsBuffer GetBufferByIndex(const SwapChain& swapChain, int index);

		/// <summary>
		/// <para>[Command]</para>
		/// ResourceBarrier() を実行し、GraphicsBuffer がどう状態遷移するかをGPUに教える
		/// </summary>
		static void CommandInvokeResourceBarrierAsTransition(
			const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer,
			GraphicsBufferState before, GraphicsBufferState after, bool useAllSubresources);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>ハンドルが指し示す RenderTarget について、以下の処理を行う</para>
		/// 出力ステージとして設定する
		/// </summary>
		static void CommandSetRTAsOutputStage(const CommandList& commandList, const DescriptorHeapHandleAtCPU& handleRTV);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>ハンドルが指し示す RenderTarget について、以下の処理を行う</para>
		/// 全体を1色でクリアする
		/// </summary>
		static void CommandClearRT(
			const CommandList& commandList, const DescriptorHeapHandleAtCPU& handleRTV, const Color& clearColor);

		/// <summary>
		/// <para>[Command]</para>
		/// RootSignature を設定する
		/// </summary>
		static void CommandSetRootSignature(const CommandList& commandList, const RootSignature& rootSignature);

		/// <summary>
		/// <para>[Command]</para>
		/// GraphicsPipelineState を設定する
		/// </summary>
		static void CommandSetGraphicsPipelineState(const CommandList& commandList, const PipelineState& graphicsPipelineState);

		/// <summary>
		/// <para>[Command]</para>
		/// DescriptorHeap 群を設定する
		/// </summary>
		static void CommandSetDescriptorHeaps(const CommandList& commandList, const std::vector<DescriptorHeap>& descriptorHeaps);

		/// <summary>
		/// <para>[Command]</para>
		/// ルートパラメータのインデックスと DescriptorHeap のハンドル (GPU) を関連付ける
		/// </summary>
		static void CommandLinkRootParameterIndexAndDescriptorHeapHandleAtGPU(
			const CommandList& commandList, const Device& device, const DescriptorHeap& descriptorHeap, DescriptorHeapType descriptorHeapType,
			int rootParameterIndex, int descriptorIndexAtGPU);

		/// <summary>
		/// <para>[Command]</para>
		/// Input Assembler : トポロジーを設定する
		/// </summary>
		/// <param name="commandList"></param>
		static void CommandIASetPrimitiveTopology(const CommandList& commandList, PrimitiveTopology primitiveTopology);

		/// <summary>
		/// <para>[Command]</para>
		/// Input Assembler : 頂点バッファーを設定する (複数セット出来る)
		/// </summary>
		static void CommandIASetVertexBuffer(const CommandList& commandList, const std::vector<VertexBufferView>& vertexBufferViews);

		/// <summary>
		/// <para>[Command]</para>
		/// Input Assembler : インデックスバッファーを設定する (1つのみセット出来る)
		/// </summary>
		static void CommandIASetIndexBuffer(const CommandList& commandList, const IndexBufferView& indexBufferView);

		/// <summary>
		/// <para>[Command]</para>
		/// Rasterizer : ビューポートとシザー矩形を設定する
		/// </summary>
		static void CommandRSSetViewportAndScissorRect(const CommandList& commandList, const ViewportScissorRect& viewportScissorRect);

		/// <summary>
		/// <para>[Command]</para>
		/// 描画命令を発行する (インスタンス数 = 1)
		/// </summary>
		static void CommandDrawInstanced(const CommandList& commandList, int vertexCount);

		/// <summary>
		/// <para>[Command]</para>
		/// 描画命令を発行する (インスタンス数 = 1)
		/// </summary>
		static void CommandDrawIndexedInstanced(const CommandList& commandList, int indexCount);

		/// <summary>
		/// <para>[Command]</para>
		/// CommandList を閉じる
		/// </summary>
		static void CommandClose(const CommandList& commandList);

		/// <summary>
		/// CommandList を実行する
		/// </summary>
		static void ExecuteCommands(const CommandQueue& commandQueue, const CommandList& commandList);

		/// <summary>
		/// <para>GPU側の処理が終わるまで、無限に同期待機する</para>
		/// エラーが起きて中断されたら false を, 正常に完了したら true を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool WaitForGPUEventCompletion(const Fence& fence, const CommandQueue& commandQueue);

		/// <summary>
		/// <para>スワップ(フリップ) を実行させる</para>
		/// 成功したら true, 失敗したら false を返す
		/// </summary>
		static bool Present(const SwapChain& swapChain);

		/// <summary>
		/// <para>シェーダーファイルをコンパイルして返す (失敗したら nullptr)</para>
		/// </summary>
		static Blob CompileShaderFile(
			const std::wstring& path, const std::string& entryFunc, const std::string& shaderTarget, std::wstring& outErrorMessage);

		/// <summary>
		/// <para>1つのファイルから VS, PS を順にコンパイルして返す (失敗したら nullptr)</para>
		/// 成功したら true, 失敗したら false を返す (失敗しても処理を最後まで行い、エラーメッセージをまとめて outErrorMessage に格納する)
		/// </summary>
		static bool CompileShaderFile_VS_PS(
			const std::wstring& path,
			const std::string& entryFuncVS, const std::string& entryFuncPS,
			Blob& outVS, Blob& outPS,
			std::wstring& outErrorMessage
		);

#ifdef _DEBUG
		/// <summary>
		/// <para>DebugLayer を有効化する</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool EnableDebugLayer();
#endif
	};
}
