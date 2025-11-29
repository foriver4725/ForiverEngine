#pragma once

struct IDXGIFactory7;
struct ID3D12Device14;
struct IDXGISwapChain4;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandQueue;
struct ID3D12DescriptorHeap;
struct ID3D12Fence;
struct IDXGIAdapter;
struct ID3D12Resource;
struct HWND__; typedef HWND__* HWND;

namespace ForiverEngine
{
	// 最新バージョンを指定
	using IDXGIFactoryLatest = IDXGIFactory7;
	using ID3D12DeviceLatest = ID3D12Device14;
	using IDXGISwapChainLatest = IDXGISwapChain4;

	// ラッパークラスを定義
#define DEFINE_POINTER_WRAPPER_STRUCT(WrapperStructName, OriginalPointerType) \
struct WrapperStructName \
{ \
public: \
	OriginalPointerType* Ptr = nullptr; \
	explicit WrapperStructName(OriginalPointerType* ptr) : Ptr(ptr) {} \
    inline static WrapperStructName Nullptr() { return WrapperStructName(nullptr); } \
\
	OriginalPointerType* operator->() const { return Ptr; } \
    explicit operator bool() const { return Ptr != nullptr; } \
};

	DEFINE_POINTER_WRAPPER_STRUCT(Factory, IDXGIFactoryLatest);
	DEFINE_POINTER_WRAPPER_STRUCT(Device, ID3D12DeviceLatest);
	DEFINE_POINTER_WRAPPER_STRUCT(SwapChain, IDXGISwapChainLatest);
	DEFINE_POINTER_WRAPPER_STRUCT(CommandAllocator, ID3D12CommandAllocator);
	DEFINE_POINTER_WRAPPER_STRUCT(CommandList, ID3D12GraphicsCommandList);
	DEFINE_POINTER_WRAPPER_STRUCT(CommandQueue, ID3D12CommandQueue);
	DEFINE_POINTER_WRAPPER_STRUCT(DescriptorHeap, ID3D12DescriptorHeap);
	DEFINE_POINTER_WRAPPER_STRUCT(Fence, ID3D12Fence);

	DEFINE_POINTER_WRAPPER_STRUCT(GraphicAdapter, IDXGIAdapter);
	DEFINE_POINTER_WRAPPER_STRUCT(GraphicBuffer, ID3D12Resource);

#undef DEFINE_POINTER_WRAPPER_STRUCT

	// D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE をメモリ配置そのままに自作したもの
	// reinterpret_cast で相互キャストし、外部翻訳単位にはこちらを公開するようにする
	typedef unsigned __int64 UINT64;
	typedef unsigned __int64 ULONG_PTR;
	typedef ULONG_PTR SIZE_T;
	struct DescriptorHeapHandleAtCPU { SIZE_T ptr; };
	struct DescriptorHeapHandleAtGPU { UINT64 ptr; };

	class D3D12ObjectFactory final
	{
	public:
		D3D12ObjectFactory() = delete;
		~D3D12ObjectFactory() = delete;

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
		/// <para>DescriptorHeap(RTV) と SwapChain を関連付ける</para>
		/// <para>Descriptorの数 = GraphicBufferの数だけ、繰り返し処理を行う</para>
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
		/// <para>SwapChain から現在バックバッファである GraphicBuffer のインデックスを取得する</para>
		/// 常に、必ず1つの GraphicBuffer がバックバッファである想定
		/// </summary>
		static int GetCurrentBackBufferIndex(const SwapChain& swapChain);

		/// <summary>
		/// SwapChain から指定インデックスの GraphicBuffer を取得する (失敗したら nullptr)
		/// </summary>
		static GraphicBuffer GetGraphicBufferByIndex(const SwapChain& swapChain, int index);

		/// <summary>
		/// DescriptorHeap (RTV) のハンドルを作成し、index 番目の Descriptor (RTV) を指し示すように内部ポインタを進めて返す
		/// </summary>
		static DescriptorHeapHandleAtCPU CreateDescriptorRTVHandleByIndex(
			const Device& device, const DescriptorHeap& descriptorHeapRTV, int index);

		/// <summary>
		/// <para>ResourceBarrier() を実行し、GraphicBuffer がどう状態遷移するかをGPUに教える</para>
		/// Present -> RenderTarget
		/// </summary>
		static void InvokeResourceBarrierAsTransitionFromPresentToRenderTarget(
			const CommandList& commandList, const GraphicBuffer& graphicBuffer);

		/// <summary>
		/// <para>ResourceBarrier() を実行し、GraphicBuffer がどう状態遷移するかをGPUに教える</para>
		/// RenderTarget -> Present
		/// </summary>
		static void InvokeResourceBarrierAsTransitionFromRenderTargetToPresent(
			const CommandList& commandList, const GraphicBuffer& graphicBuffer);

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
		static void CommandClearRT(const CommandList& commandList, const DescriptorHeapHandleAtCPU& handleRTV, float clearColor4[]);

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

#ifdef _DEBUG
		/// <summary>
		/// <para>DebugLayer を有効化する</para>
		/// 成功したら true, 失敗したら false を返す (失敗した瞬間に処理を中断する)
		/// </summary>
		static bool EnableDebugLayer();
#endif
	};
}
