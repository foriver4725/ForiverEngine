#pragma once

struct IDXGIFactory7;
struct ID3D12Device14;
struct IDXGISwapChain4;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12CommandQueue;
struct IDXGIAdapter;
struct HWND__; typedef HWND__* HWND;

namespace ForiverEngine
{
	// 最新バージョンを指定
	using IDXGIFactoryLatest = IDXGIFactory7;
	using ID3D12DeviceLatest = ID3D12Device14;
	using IDXGISwapChainLatest = IDXGISwapChain4;

	// ラッパークラスを定義
#define DEFINE_WRAPPER_CLASS(WrapperStructName, OriginalPointerType) \
struct WrapperStructName \
{ \
public: \
	OriginalPointerType* Ptr = nullptr; \
	explicit WrapperStructName(OriginalPointerType* ptr) : Ptr(ptr) {} \
    inline bool IsValid() const { return Ptr != nullptr; } \
    inline static WrapperStructName Nullptr() { return WrapperStructName(nullptr); } \
};

	DEFINE_WRAPPER_CLASS(Factory, IDXGIFactoryLatest);
	DEFINE_WRAPPER_CLASS(Device, ID3D12DeviceLatest);
	DEFINE_WRAPPER_CLASS(SwapChain, IDXGISwapChainLatest);
	DEFINE_WRAPPER_CLASS(CommandAllocator, ID3D12CommandAllocator);
	DEFINE_WRAPPER_CLASS(CommandList, ID3D12GraphicsCommandList);
	DEFINE_WRAPPER_CLASS(CommandQueue, ID3D12CommandQueue);

	DEFINE_WRAPPER_CLASS(GraphicAdapter, IDXGIAdapter);

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
	};
}
