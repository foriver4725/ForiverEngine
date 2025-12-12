// 前方宣言やオブジェクトの定義などを行う
// DirectX12 は直接インクルードしない!

#pragma once

#include <scripts/common/Include.h>

// 前方宣言

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

namespace ForiverEngine
{
	// シェーダーターゲットを指定
#define ShaderTargetVS "vs_5_1"
#define ShaderTargetPS "ps_5_1"

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

	enum class PrimitiveTopology : std::uint8_t
	{
		PointList = 1,    // D3D_PRIMITIVE_TOPOLOGY_POINTLIST
		LineList = 2,     // D3D_PRIMITIVE_TOPOLOGY_LINELIST
		TriangleList = 4, // D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
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
}
