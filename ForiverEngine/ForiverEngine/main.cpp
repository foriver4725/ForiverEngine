//#define ENABLE_CUI_CONSOLE

#include "scripts/headers/D3D12BasicFlow.h"

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

BEGIN_INITIALIZE(L"DX12Sample", L"DX12 テスト", hwnd, WindowWidth, WindowHeight);
{
	using namespace ForiverEngine;

	//Print("Hello, World!\n");

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	WindowHelper::SetTargetFps(60);

	const auto [factory, device, commandAllocator, commandList, commandQueue, swapChain]
		= D3D12BasicFlow::CreateStandardObjects(hwnd, WindowWidth, WindowHeight);

	const RootParameter rootParameter = RootParameter::CreateBasic(1, 1, 0);
	const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Wrap, Filter::Point);
	const auto [shaderVS, shaderPS] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");
	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayouts, FillMode::Solid, CullMode::None);

	const auto [rtBufferGetter, rtvGetter] = D3D12BasicFlow::InitRTV(device, swapChain, 2, false);
	const DescriptorHeapHandleAtCPU dsv = D3D12BasicFlow::InitDSV(device, WindowWidth, WindowHeight, 1.0f);

	const Mesh mesh = Mesh::CreateCube();

	Transform transform =
	{
		.parent = nullptr,
		.position = Vector3::Zero(),
		.rotation = Quaternion::Identity(),
		.scale = Vector3::One(),
	};

	CameraTransform cameraTransform =
	{
		.position = Vector3(0, 3, -5),
		.target = Vector3::Zero(),
		.up = Vector3::Up(),
		.nearClip = 0.1f,
		.farClip = 1000.0f,
		.isPerspective = true,
		.fov = 60.0f * DegToRad,
		.aspectRatio = 1.0f * WindowWidth / WindowHeight,
	};

	// b0 レジスタに渡すデータ
	struct alignas(256) CBData0
	{
		Matrix4x4 Matrix_M_IT; // M の逆→転置行列
		Matrix4x4 Matrix_MVP; // MVP
		int TextureIndex; // 使用するテクスチャのインデックス (仮)
		int UseUpperUV; // 上半分のUVを使うかどうか (仮). 1=true:上半分, 0=false:下半分
	};

	CBData0 cbData0 =
	{
		.Matrix_M_IT = transform.CalculateModelMatrixInversed().Transposed(),
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(transform, cameraTransform),
		.TextureIndex = 0,
		.UseUpperUV = true,
	};

	// CBV 用バッファ
	CBData0* cbData0VirtualPtr = nullptr;
	const GraphicsBuffer cbvBuffer = D3D12BasicFlow::InitCBVBuffer<CBData0>(device, cbData0, false, &cbData0VirtualPtr);

	// SRV 用バッファ
	const auto srvBufferAndData = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue,
		{
			"assets/textures/grass_stone.png",
			"assets/textures/dirt_sand.png",
		});
	const Texture textureArrayMetadata = std::get<1>(srvBufferAndData);

	// DescriptorHeap に登録
	const DescriptorHeap descriptorHeapBasic
		= D3D12BasicFlow::InitDescriptorHeapBasic(device, { cbvBuffer }, { srvBufferAndData });

	const auto [vertexBufferView, indexBufferView]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, mesh);

	const ViewportScissorRect viewportScissorRect
		= ViewportScissorRect::CreateFullSized(WindowWidth, WindowHeight);

	BEGIN_FRAME;
	{
#if true
		// 適当に、立方体を回転させる
		if (InputHelper::GetKeyInfo(Key::Space).pressed)
		{
			transform.rotation = Quaternion::FromAxisAngle(Vector3::Up(), 180.0f * DegToRad * WindowHelper::GetDeltaSeconds()) * transform.rotation;
			cbData0VirtualPtr->Matrix_M_IT = transform.CalculateModelMatrixInversed().Transposed();
			cbData0VirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(transform, cameraTransform);
		}
		// 適当に、テクスチャを切り替える
		if (InputHelper::GetKeyInfo(Key::N1).pressedNow)
		{
			cbData0VirtualPtr->TextureIndex = (cbData0VirtualPtr->TextureIndex + 1) % textureArrayMetadata.sliceCount;
		}
		// 適当に、使うUVを切り替える
		if (InputHelper::GetKeyInfo(Key::N2).pressedNow)
		{
			cbData0VirtualPtr->UseUpperUV = 1 - cbData0VirtualPtr->UseUpperUV;
		}
#endif

		const int currentBackBufferIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);
		const GraphicsBuffer currentBackBuffer = rtBufferGetter(currentBackBufferIndex);
		const DescriptorHeapHandleAtCPU currentBackBufferRTV = rtvGetter(currentBackBufferIndex);
		if (!currentBackBuffer)
			ShowError(L"現在のバックバッファーの取得に失敗しました");

		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignature, graphicsPipelineState, currentBackBuffer,
			currentBackBufferRTV, dsv, descriptorHeapBasic, { vertexBufferView }, indexBufferView,
			viewportScissorRect, PrimitiveTopology::TriangleList, Color::Black(), 1.0f,
			static_cast<int>(mesh.indices.size())
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_FRAME;
}
END_INITIALIZE(0);
