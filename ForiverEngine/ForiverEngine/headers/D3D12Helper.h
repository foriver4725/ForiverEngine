#pragma once

// 一般的なものはヘッダでインクルードする (必要最小限)
// DirectX12 はソースでのみインクルードする
#include <vector>
#include <array>
#include <string>
#include <functional>
#include "Windows.h"

struct IDXGIFactory7;
struct ID3D12Device14;
struct IDXGISwapChain4;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandQueue;
struct ID3D12DescriptorHeap;
struct ID3D12Fence;
struct IDXGIAdapter;
struct ID3D12Resource;
struct ID3D10Blob; typedef ID3D10Blob ID3DBlob;
struct HWND__; typedef HWND__* HWND;

// 最新バージョンを指定
using IDXGIFactoryLatest = IDXGIFactory7;
using ID3D12DeviceLatest = ID3D12Device14;
using IDXGISwapChainLatest = IDXGISwapChain4;

// シェーダーターゲットを指定
#define ShaderTargetVS "vs_5_1"
#define ShaderTargetPS "ps_5_1"

namespace ForiverEngine
{
	// DirectX と等価の列挙型を定義

	// ピクセルフォーマット
	enum class Format : int
	{
		Unknown = 0,     // DXGI_FORMAT_UNKNOWN

		RGBA_F32 = 2,    // DXGI_FORMAT_R32G32B32A32_FLOAT
		RGBA_U32 = 3,    // DXGI_FORMAT_R32G32B32A32_UINT
		RGB_F32 = 6,     // DXGI_FORMAT_R32G32B32_FLOAT
		RGB_U32 = 7,     // DXGI_FORMAT_R32G32B32_UINT
		RG_F32 = 16,     // DXGI_FORMAT_R32G32_FLOAT
		RG_U32 = 17,     // DXGI_FORMAT_R32G32_UINT
		R_F32 = 41,      // DXGI_FORMAT_R32_FLOAT
		R_U32 = 42,      // DXGI_FORMAT_R32_UINT

		RGBA_F16 = 10,   // DXGI_FORMAT_R16G16B16A16_FLOAT
		RGBA_U16 = 12,   // DXGI_FORMAT_R16G16B16A16_UINT
		RG_F16 = 34,     // DXGI_FORMAT_R16G16_FLOAT
		RG_U16 = 36,     // DXGI_FORMAT_R16G16_UINT
		R_F16 = 54,      // DXGI_FORMAT_R16_FLOAT
		R_U16 = 57,      // DXGI_FORMAT_R16_UINT

		RGBA_U8 = 30,    // DXGI_FORMAT_R8G8B8A8_UINT
		RGBA_U8_01 = 28, // DXGI_FORMAT_R8G8B8A8_UNORM

		D_F32 = 40,      // DXGI_FORMAT_D32_FLOAT
	};

	// ラスタライザの塗りつぶしモード
	struct FillMode
	{
		static constexpr int Wireframe = 2; // D3D12_FILL_MODE_WIREFRAME
		static constexpr int Solid = 3;     // D3D12_FILL_MODE_SOLID
	};

	// ラスタライザのカリングモード
	struct CullMode
	{
		static constexpr int None = 1;  // D3D12_CULL_MODE_NONE
		static constexpr int Front = 2; // D3D12_CULL_MODE_FRONT
		static constexpr int Back = 3;  // D3D12_CULL_MODE_BACK
	};

	// ラッパークラスを定義
#define DEFINE_POINTER_WRAPPER_STRUCT(WrapperStructName, OriginalPointerType) \
struct WrapperStructName \
{ \
public: \
	OriginalPointerType* Ptr = nullptr; \
\
	explicit WrapperStructName() : Ptr(nullptr) {} \
	explicit WrapperStructName(OriginalPointerType* ptr) : Ptr(ptr) {} \
\
	OriginalPointerType* operator->() const { return Ptr; } \
    explicit operator bool() const { return Ptr != nullptr; } \
};

	DEFINE_POINTER_WRAPPER_STRUCT(Factory, IDXGIFactoryLatest);
	DEFINE_POINTER_WRAPPER_STRUCT(Device, ID3D12DeviceLatest);
	DEFINE_POINTER_WRAPPER_STRUCT(SwapChain, IDXGISwapChainLatest);
	DEFINE_POINTER_WRAPPER_STRUCT(RootSignature, ID3D12RootSignature);
	DEFINE_POINTER_WRAPPER_STRUCT(PipelineState, ID3D12PipelineState);
	DEFINE_POINTER_WRAPPER_STRUCT(CommandAllocator, ID3D12CommandAllocator);
	DEFINE_POINTER_WRAPPER_STRUCT(CommandList, ID3D12GraphicsCommandList);
	DEFINE_POINTER_WRAPPER_STRUCT(CommandQueue, ID3D12CommandQueue);
	DEFINE_POINTER_WRAPPER_STRUCT(DescriptorHeap, ID3D12DescriptorHeap);
	DEFINE_POINTER_WRAPPER_STRUCT(Fence, ID3D12Fence);

	DEFINE_POINTER_WRAPPER_STRUCT(GraphicAdapter, IDXGIAdapter);
	DEFINE_POINTER_WRAPPER_STRUCT(GraphicsBuffer, ID3D12Resource); // GPUメモリを指し示す (フレームバッファとかもそう)
	DEFINE_POINTER_WRAPPER_STRUCT(Blob, ID3DBlob);

#undef DEFINE_POINTER_WRAPPER_STRUCT

	// 頂点レイアウト 単品
	struct VertexLayout
	{
		const char* SemanticName;
		Format Format;
	};

	// ビューポートとシザー矩形
	// どちらも同じサイズで設定される
	struct ViewportScissorRect
	{
		int minX, maxX;
		int minY, maxY;
	};

	// DirectX の構造体を直接外部に公開したくないので、メモリ配置を同じにした構造体に reinterpret_cast する
	typedef ULONG_PTR SIZE_T;
	struct DescriptorHeapHandleAtCPU { SIZE_T ptr; };
	struct DescriptorHeapHandleAtGPU { UINT64 ptr; };
	struct VertexBufferView { UINT64 bufferAddress; UINT verticesSize; UINT vertexSize; };
	struct IndexBufferView { UINT64 bufferAddress; UINT indicesSize; Format indexFormat; };

	class D3D12Helper final
	{
	public:
		D3D12Helper() = delete;
		~D3D12Helper() = delete;

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
		static RootSignature CreateRootSignature(const Device& device, std::wstring& outErrorMessage);

		/// <summary>
		/// GraphicsPipelineState を作成して返す (失敗したら nullptr)
		/// </summary>
		static PipelineState CreateGraphicsPipelineState(
			const Device& device, const RootSignature& rootSignature, const Blob& vs, const Blob& ps,
			const std::vector<VertexLayout>& vertexLayouts, int eFillMode, int eCullMode);

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
		/// <para>DescriptorHeap を作成して返す (失敗したら nullptr)</para>
		/// RenderTargetView 用
		/// </summary>
		static DescriptorHeap CreateDescriptorHeapRTV(const Device& device);

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
		/// <para>GraphicsBuffer の Map() を使って、CPUのバッファをGPU側にコピーする</para>
		/// <para>バッファのサイズは、GraphicsBuffer 作成時に指定したサイズと同じであること! (一部のバッファのみコピー、などには未対応)</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool CopyDataFromCPUToGPUThroughGraphicsBuffer(const GraphicsBuffer& GraphicsBuffer, void* dataBegin, int dataSize);

		/// <summary>
		/// <para>DescriptorHeap(RTV) と SwapChain を関連付ける</para>
		/// <para>Descriptorの数 = バッファの数だけ、繰り返し処理を行う</para>
		/// 全て成功したら true, 1つでも失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool LinkDescriptorHeapRTVToSwapChain(
			const Device& device, const DescriptorHeap& descriptorHeapRTV, const SwapChain& swapChain);

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
		/// DescriptorHeap (RTV) のハンドルを作成し、index 番目の Descriptor (RTV) を指し示すように内部ポインタを進めて返す
		/// </summary>
		static DescriptorHeapHandleAtCPU CreateDescriptorRTVHandleByIndex(
			const Device& device, const DescriptorHeap& descriptorHeapRTV, int index);

		/// <summary>
		/// <para>ResourceBarrier() を実行し、GraphicsBuffer がどう状態遷移するかをGPUに教える</para>
		/// Present -> RenderTarget
		/// </summary>
		static void InvokeResourceBarrierAsTransitionFromPresentToRenderTarget(
			const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer);

		/// <summary>
		/// <para>ResourceBarrier() を実行し、GraphicsBuffer がどう状態遷移するかをGPUに教える</para>
		/// RenderTarget -> Present
		/// </summary>
		static void InvokeResourceBarrierAsTransitionFromRenderTargetToPresent(
			const CommandList& commandList, const GraphicsBuffer& GraphicsBuffer);

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
			const CommandList& commandList, const DescriptorHeapHandleAtCPU& handleRTV, const std::array<float, 4>& clearColor);

		/// <summary>
		/// <para>[Command]</para>
		/// GraphicsPipelineState を設定する
		/// </summary>
		static void CommandSetGraphicsPipelineState(const CommandList& commandList, const PipelineState& graphicsPipelineState);

		/// <summary>
		/// <para>[Command]</para>
		/// RootSignature を設定する
		/// </summary>
		static void CommandSetRootSignature(const CommandList& commandList, const RootSignature& rootSignature);

		/// <summary>
		/// <para>[Command]</para>
		/// Input Assembler : トポロジーを三角形リストに設定する
		/// </summary>
		/// <param name="commandList"></param>
		static void CommandIASetTopologyAsTriangleList(const CommandList& commandList);

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

#ifdef _DEBUG
		/// <summary>
		/// <para>DebugLayer を有効化する</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool EnableDebugLayer();
#endif
	};
}
