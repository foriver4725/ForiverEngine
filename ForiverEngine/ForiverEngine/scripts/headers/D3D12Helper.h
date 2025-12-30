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
			const std::vector<VertexLayout>& vertexLayouts, FillMode fillMode, CullMode cullMode, bool useDSV);

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
		/// <para>2次元テクスチャ/2次元テクスチャ配列 用 (テクスチャのタイプが2Dで無いならば、失敗とみなし nullptr を返す)</para>
		/// <para>GPU内でのみ用いる想定で、CPUからのマップ不可</para>
		/// <para>テクスチャ変数のメタデータを基に作成する</para>
		/// </summary>
		static GraphicsBuffer CreateGraphicsBufferTexture2D(const Device& device, const Texture& texture);

		/// <summary>
		/// <para>GPU側のメモリ領域を確保し、その GraphicsBuffer を返す (失敗したら nullptr)</para>
		/// <para>2次元テクスチャ用</para>
		/// <para>オフスクリーンレンダリング用で、レンダーターゲット (RT) とシェーダーリソース (SR) で切り替えて用いる</para>
		/// <para>sRGB の設定不可. 最初は SR として作成する</para>
		/// <para>GPU内でのみ用いる想定で、CPUからのマップ不可</para>
		/// </summary>
		static GraphicsBuffer CreateGraphicsBufferTexture2DForRTAndSR(const Device& device, int width, int height, const Color& clearValue);

		/// <summary>
		/// <para>GPU側のメモリ領域を確保し、その GraphicsBuffer を返す (失敗したら nullptr)</para>
		/// <para>デプスバッファ用 (ステンシルは用いず、32bit 深度のみとする)</para>
		/// <para>GPU内でのみ用いる想定で、CPUからのマップ不可</para>
		/// </summary>
		static GraphicsBuffer CreateGraphicsBufferTexture2DAsDepthBuffer(const Device& device, int width, int height, float clearValue);

		/// <summary>
		/// <para>RTV を作成し、RTV 用 DescriptorHeap に登録する (基本)</para>
		/// <para>swapChain からレンダーターゲットバッファ群を取得し、それぞれに対して RTV を作成する</para>
		/// <para>全て成功したら true, 1つでも失敗したら false を返す (失敗した瞬間に処理を中断する)<para>
		/// <para>戻り値として取得することは出来ない!</para>
		/// </summary>
		static bool CreateRenderTargetViews(
			const Device& device, const DescriptorHeap& descriptorHeapRTV, const SwapChain& swapChain, bool sRGB);

		/// <summary>
		/// <para>RTV を作成し、RTV 用 DescriptorHeap に登録する (ポストプロセス用)</para>
		/// <para>RT を基に RTV を作成し、 DescriptorHeap の index 番目に登録する</para>
		/// <para>1つだけ作成する. sRGB 不可.</para>
		/// <para>戻り値として取得することは出来ない!</para>
		/// </summary>
		static void CreateRenderTargetViewPP(
			const Device& device, const DescriptorHeap& descriptorHeapRTV, const GraphicsBuffer& rt, int index);

		/// <summary>
		/// <para>DSV を作成し、DSV 用 DescriptorHeap に登録する</para>
		/// <para>ステンシルは用いず、32bit 深度のみとする</para>
		/// <para>戻り値として取得することは出来ない!</para>
		/// </summary>
		static void CreateDepthStencilView(
			const Device& device, const DescriptorHeap& descriptorHeapDSV, const GraphicsBuffer& depthBuffer);

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
		/// <para>テクスチャのスライス数が1より大きい場合、2Dテクスチャ配列として登録する</para>
		/// 戻り値として取得することは出来ない!
		/// </summary>
		static void CreateSRVAndRegistToDescriptorHeap(
			const Device& device, const DescriptorHeap& descriptorHeap, const GraphicsBuffer& graphicsBuffer, int index,
			const Texture& textureAsMetadata);

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
		/// <para>Unmap() しないように設定した場合、outBufferVirtualPtr を経由してポインタが返される</para>
		/// <para>そのポインタを通じてデータを書き換えることで、シェーダーから参照しているメモリの内容を書き換えることが出来る</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool CopyDataFromCPUToGPUThroughGraphicsBuffer1D(
			const GraphicsBuffer& graphicsBuffer, const void* dataBegin, int dataSize,
			bool unmapOnEnd = true, void** outBufferVirtualPtr = nullptr);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>CPUのバッファをGPU側にコピーする</para>
		/// <para>2次元テクスチャ/2次元テクスチャ配列 用 (テクスチャのタイプが2Dで無いならば、失敗とみなし false を返す)</para>
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
		/// <para>RTV, DSV を、RT として出力ステージに設定する</para>
		/// <para>DSV について、nullptr が渡された場合、DSV を設定しない</para>
		/// </summary>
		static void CommandSetRT(const CommandList& commandList,
			const DescriptorHeapHandleAtCPU& rtv, const DescriptorHeapHandleAtCPU& dsv);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>RT をクリアする</para>
		/// <para>DSV について、ステンシルは使っていないので、深度のみのクリアをしている</para>
		/// <para>DSV について、nullptr が渡された場合、DSV をクリアしない</para>
		/// </summary>
		static void CommandClearRT(
			const CommandList& commandList, const DescriptorHeapHandleAtCPU& rtv, const DescriptorHeapHandleAtCPU& dsv,
			Color rtvClearValue, float dsvClearValue);

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
		/// <para>DescriptorHeap を RootSignature にリンクする</para>
		/// DescriptorHeap の最初のハンドルを受け取り、それを rootParameterIndex 番目のルートパラメータにリンクする
		/// </summary>
		static void CommandLinkDescriptorHeapToRootSignature(
			const CommandList& commandList, const DescriptorHeapHandleAtGPU& firstDescriptor, int rootParameterIndex);

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
		/// <para>[Command]</para>
		/// <para>コマンドリストをクローズして実行し、GPUの処理が完了するまで待機する</para>
		/// エラーが起きて中断されたら false を, 正常に完了したら true を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool CommandCloseAndWaitForCompletion(const Device& device, const CommandQueue& commandQueue, const CommandList& commandList);

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

		//////////////////////////////////////////////////
		// テキスト表示

		// シングルトンオブジェクトを作る必要がある
	private:
		inline static bool hasInitializedText = false;
		inline static DirectX::DX12::GraphicsMemory* textGraphicsMemory = nullptr;
		inline static DirectX::DX12::SpriteBatch* textSpriteBatch = nullptr;
		inline static DirectX::DX12::SpriteFont* textSpriteFont = nullptr;

	public:

		/// <summary>
		/// <para>テキストの初期化</para>
		/// <para>最初に1回だけ呼び出すこと</para>
		/// テキスト用に作成された DescriptorHeap を返す (失敗したら nullptr)
		/// </summary>
		static DescriptorHeap InitText(
			const Device& device, const ViewportScissorRect& viewportScissorRect,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const std::string& fontPath
		);

		/// <summary>
		/// <para>[Command]</para>
		/// テキスト描画 (RegistTextDraw) の開始前に実行すること
		/// </summary>
		static void CommandBeginTextDraw(const DescriptorHeap& descriptorHeap, const CommandList& commandList);

		/// <summary>
		/// テキスト描画を予約する (テキスト描画で呼び出すもの)
		/// </summary>
		static void RegistTextDraw(const std::string& text, const Lattice2& position, const Color& color);

		/// <summary>
		/// テキスト描画 (RegistTextDraw) の終了後に実行すること
		/// </summary>
		static void EndTextDraw();

		/// <summary>
		/// <para>全てのコマンドを実行後に呼び出すこと</para>
		/// <para>Present() の直後とかが望ましい</para>
		/// メモリリークを防止するため
		/// </summary>
		/// <param name="commandQueue"></param>
		static void FinalizeTextDrawAfterCommandExecution(const CommandQueue& commandQueue);

		//////////////////////////////////////////////////

#ifdef _DEBUG
		/// <summary>
		/// <para>DebugLayer を有効化する</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool EnableDebugLayer();
#endif
	};
}
