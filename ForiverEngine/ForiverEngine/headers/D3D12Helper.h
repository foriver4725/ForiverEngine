#pragma once

// 一般的なものはヘッダでインクルードする (必要最小限)
// DirectX12 はソースでのみインクルードする

#include "./Vector.h"
#include "./StringUtils.h"

#include <string>
#include <vector>
#include <tuple>
#include <array>
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
	enum class Format : std::uint8_t
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
	enum class FillMode : std::uint8_t
	{
		Wireframe = 2, // D3D12_FILL_MODE_WIREFRAME
		Solid = 3,     // D3D12_FILL_MODE_SOLID
	};

	// ラスタライザのカリングモード
	enum class CullMode : std::uint8_t
	{
		None = 1,  // D3D12_CULL_MODE_NONE
		Front = 2, // D3D12_CULL_MODE_FRONT
		Back = 3,  // D3D12_CULL_MODE_BACK
	};

	// シェーダーに公開する範囲 = どのシェーダーから参照可能か
	enum class ShaderVisibility : std::uint8_t
	{
		All = 0,        // D3D12_SHADER_VISIBILITY_ALL
		VertexOnly = 1, // D3D12_SHADER_VISIBILITY_VERTEX
		PixelOnly = 5,  // D3D12_SHADER_VISIBILITY_PIXEL
	};

	// DescriptorHeap の種類
	enum class DescriptorHeapType : std::uint8_t
	{
		CBV_SRV_UAV = 0, // D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
		Sampler = 1,     // D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
		RTV = 2,         // D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		DSV = 3,         // D3D12_DESCRIPTOR_HEAP_TYPE_DSV
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

	// DirectX の構造体を直接外部に公開したくないので、データを自作して、内部処理で構築する

	// ルートパラメータ
	struct RootParameter
	{
		// DirectX と等価の列挙型を定義
		// 混同されそうなので、この構造体内で定義してしまう

		enum class DescriptorRangeType : std::uint8_t
		{
			SRV = 0,     // D3D12_DESCRIPTOR_RANGE_TYPE_SRV
			UAV = 1,     // D3D12_DESCRIPTOR_RANGE_TYPE_UAV
			CBV = 2,     // D3D12_DESCRIPTOR_RANGE_TYPE_CBV
			Sampler = 3, // D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
		};

		struct DescriptorRange
		{
			DescriptorRangeType type;
			int amount;
			int registerIndex; // t#, u#, b#, s# のレジスタに登録される
		};

		ShaderVisibility shaderVisibility;
		std::vector<DescriptorRange> descriptorRanges; // この順に登録される
	};

	// サンプラーの設定
	struct SamplerConfig
	{
		// DirectX と等価の列挙型を定義
		// 命名的に混同されそうなので、この構造体内で定義してしまう

		enum class AddressingMode : std::uint8_t
		{
			Wrap = 1,       // D3D12_TEXTURE_ADDRESS_MODE_WRAP
			Mirror = 2,     // D3D12_TEXTURE_ADDRESS_MODE_MIRROR
			Clamp = 3,      // D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		};

		enum class Filter : std::uint8_t
		{
			Point = 0,          // D3D12_FILTER_MIN_MAG_MIP_POINT
			Bilinear = 0x15,      // D3D12_FILTER_MIN_MAG_MIP_LINEAR
		};

		ShaderVisibility shaderVisibility;
		AddressingMode addressingMode;
		Filter filter;
		int registerIndex; // s# のレジスタに登録される
	};

	// 頂点データ 単品
	// 頂点は時計回り!!
	struct VertexData
	{
		ForiverEngine::Vector4 pos; // 左手系 X-右, Y-上, Z-奥
		ForiverEngine::Vector2 uv; // 左上が原点
	};

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

		static constexpr ViewportScissorRect CreateFullSized(int width, int height)
		{
			return ViewportScissorRect
			{
				.minX = 0, .maxX = width,
				.minY = 0, .maxY = height,
			};
		}
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
		/// <para>DescriptorHeap を作成して返す (失敗したら nullptr)</para>
		/// RenderTargetView 用
		/// </summary>
		static DescriptorHeap CreateDescriptorHeapRTV(const Device& device);

		/// <summary>
		/// <para>DescriptorHeap を作成して返す (失敗したら nullptr)</para>
		/// ShaderResourceView 用
		/// </summary>
		static DescriptorHeap CreateDescriptorHeapSRV(const Device& device, int count);

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
		/// 2次元テクスチャ用
		/// </summary>
		static GraphicsBuffer CreateGraphicsBufferTexture2D(const Device& device, int width, int height, Format format);

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
		/// <para>テクスチャバッファ から シェーダーリソースビュー を作成し、DescriptorHeap に登録する</para>
		/// 戻り値として取得することは出来ない!
		/// </summary>
		static void CreateShaderResourceViewAndRegistToDescriptorHeap(
			const GraphicsBuffer& graphicsBuffer, Format format, const Device& device, const DescriptorHeap& descriptorHeapSRV);

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
		/// <para>バッファのサイズは、GraphicsBuffer 作成時に指定したサイズと同じであること! (一部のバッファのみコピー、などには未対応)</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool CopyDataFromCPUToGPUThroughGraphicsBuffer(
			const GraphicsBuffer& graphicsBuffer, void* dataBegin, int dataSize);

		/// <summary>
		/// <para>GraphicsBuffer の WriteToSubresource() を使って、CPUのバッファをGPU側にコピーする</para>
		/// <para>バッファのサイズは、GraphicsBuffer 作成時に指定したサイズと同じであること! (一部のバッファのみコピー、などには未対応)</para>
		/// <para>成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)</para>
		/// 一部の状況でパフォーマンスが極端に低下するとのこと、注意!
		/// </summary>
		static bool CopyDataFromCPUToGPUThroughGraphicsBufferUsingWriteToSubresource(
			const GraphicsBuffer& graphicsBuffer, void* dataBegin, int dataWidth, int dataHeight);

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
