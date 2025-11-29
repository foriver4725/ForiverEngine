#pragma once

struct IDXGIFactory7;
struct ID3D12Device14;
struct IDXGISwapChain4;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandQueue;
struct ID3D12DescriptorHeap;
struct IDXGIAdapter;
struct ID3D12Resource;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
enum D3D12_DESCRIPTOR_HEAP_TYPE;
struct HWND__; typedef HWND__* HWND;

namespace ForiverEngine
{
	// 最新バージョンを指定
	using IDXGIFactoryLatest = IDXGIFactory7;
	using ID3D12DeviceLatest = ID3D12Device14;
	using IDXGISwapChainLatest = IDXGISwapChain4;

	using DescriptorHeapType = D3D12_DESCRIPTOR_HEAP_TYPE;
	using DescriptorHeapHandle = D3D12_CPU_DESCRIPTOR_HANDLE;

	// ラッパークラスを定義
#define DEFINE_WRAPPER_CLASS(WrapperStructName, OriginalPointerType) \
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

	DEFINE_WRAPPER_CLASS(Factory, IDXGIFactoryLatest);
	DEFINE_WRAPPER_CLASS(Device, ID3D12DeviceLatest);
	DEFINE_WRAPPER_CLASS(SwapChain, IDXGISwapChainLatest);
	DEFINE_WRAPPER_CLASS(CommandAllocator, ID3D12CommandAllocator);
	DEFINE_WRAPPER_CLASS(CommandList, ID3D12GraphicsCommandList);
	DEFINE_WRAPPER_CLASS(CommandQueue, ID3D12CommandQueue);
	DEFINE_WRAPPER_CLASS(DescriptorHeap, ID3D12DescriptorHeap);

	DEFINE_WRAPPER_CLASS(GraphicAdapter, IDXGIAdapter);
	DEFINE_WRAPPER_CLASS(GraphicBuffer, ID3D12Resource);

#undef DEFINE_WRAPPER_CLASS

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

		// DescriptorHeap のハンドルを作成し、index 番目の Descriptor を指し示すように内部ポインタを進めて返す
		// CPU 用
		static DescriptorHeapHandle CreateDescriptorHeapHandleIndicatingDescriptorByIndexAtCPU(
			const Device& device, const DescriptorHeap& descriptorHeap, DescriptorHeapType descriptorHeapType, int index);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>ハンドルが指し示す RenderTarget について、以下の処理を行う</para>
		/// 出力ステージとして設定する
		/// </summary>
		static void CommandSetRTAsOutputStage(const CommandList& commandList, const DescriptorHeapHandle& handleRTV);

		/// <summary>
		/// <para>[Command]</para>
		/// <para>ハンドルが指し示す RenderTarget について、以下の処理を行う</para>
		/// 全体を1色でクリアする
		/// </summary>
		static void CommandClearRT(const CommandList& commandList, const DescriptorHeapHandle& handleRTV, float clearColor4[]);

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
