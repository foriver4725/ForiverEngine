//#define ENABLE_CUI_CONSOLE

#include "scripts/headers/D3D12BasicFlow.h"

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

BEGIN_INITIALIZE(L"DX12Sample", L"DX12 テスト", hwnd, WindowWidth, WindowHeight);
{
	using namespace ForiverEngine;

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	const auto [factory, device, commandAllocator, commandList, commandQueue, swapChain]
		= D3D12BasicFlow::CreateStandardObjects(hwnd, WindowWidth, WindowHeight);

	// ダブルバッファリングなので、2つ RTV を確保する
	const DescriptorHeap descriptorHeapRTV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::RTV, 2, false);
	if (!descriptorHeapRTV)
		ShowError(L"DescriptorHeap (RTV) の作成に失敗しました");
	if (!D3D12Helper::CreateRenderTargetViews(device, descriptorHeapRTV, swapChain, false))
		ShowError(L"RenderTargetView を作成できない RenderTargetBuffer がありました");

	// 深度バッファは手動で生成する必要がある
	// 記録用なので、1つで十分
	const GraphicsBuffer depthBuffer = D3D12Helper::CreateGraphicsBufferTexture2DAsDepthBuffer(device, WindowWidth, WindowHeight, 1.0f);
	if (!depthBuffer)
		ShowError(L"DepthBuffer の作成に失敗しました");
	// DSVは異なるカテゴリなので、別個に DescriptorHeap を作成する
	const DescriptorHeap descriptorHeapDSV = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::DSV, 1, false);
	if (!descriptorHeapDSV)
		ShowError(L"DescriptorHeap (DSV) の作成に失敗しました");
	D3D12Helper::CreateDepthStencilView(device, descriptorHeapDSV, depthBuffer);
	// スワップしないので、ここで Descriptor のハンドルを取得しておけばよい
	const DescriptorHeapHandleAtCPU dsv = D3D12Helper::CreateDescriptorHeapHandleAtCPUIndicatingDescriptorByIndex(
		device, descriptorHeapDSV, DescriptorHeapType::DSV, 0);

	// ルートパラメータ
	const RootParameter rootParameter =
	{
		.shaderVisibility = ShaderVisibility::All,
		.descriptorRanges =
		{
			{
				.type = RootParameter::DescriptorRangeType::CBV,
				.amount = 1,
				.shaderRegister = ShaderRegister::b0,
			},
			{
				.type = RootParameter::DescriptorRangeType::SRV,
				.amount = 1,
				.shaderRegister = ShaderRegister::t0,
			},
		}
	};

	const SamplerConfig samplerConfig =
	{
		.shaderVisibility = ShaderVisibility::PixelOnly,
		.addressingMode = SamplerConfig::AddressingMode::Wrap,
		.filter = SamplerConfig::Filter::Point,
		.shaderRegister = ShaderRegister::s0,
	};

	// b0 レジスタに渡すデータ
	struct alignas(256) CBData0
	{
		Matrix4x4 Matrix_M_IT; // M の逆→転置行列
		Matrix4x4 Matrix_MVP; // MVP
	};

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

	CBData0 cbData0 =
	{
		.Matrix_M_IT = transform.CalculateModelMatrixInversed().Transposed(),
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(transform, cameraTransform),
	};

	// 定数バッファー (サイズは256バイトにアラインメントする必要がある!!)
	const GraphicsBuffer constantBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, GetAlignmentedSize(sizeof(cbData0), 256), true);
	if (!constantBuffer)
		ShowError(L"定数バッファーの作成に失敗しました");
	// 定数バッファーにデータを書き込む
	// Unmap しないでおく
	CBData0* constantBufferVirtualPtr = nullptr;
	if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(constantBuffer, &cbData0, sizeof(cbData0),
		false, reinterpret_cast<void**>(&constantBufferVirtualPtr)))
		ShowError(L"定数バッファーへのデータ転送に失敗しました");

	const Texture texture = AssetLoader::LoadTexture("assets/textures/grass.png");
	if (!texture.IsValid())
		ShowError(L"テクスチャのロードに失敗しました");
	const GraphicsBuffer textureBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, texture);
	if (!textureBuffer)
		ShowError(L"テクスチャバッファの作成に失敗しました");
	D3D12BasicFlow::UploadTextureToGPU(commandList, commandQueue, device, textureBuffer, texture);

	// CBV, SRV から成る DescriptorHeap
	const DescriptorHeap descriptorHeapBasic = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::CBV_SRV_UAV, 2, true);
	if (!descriptorHeapBasic)
		ShowError(L"CBV/SRV/UAV 用 DescriptorHeap の作成に失敗しました");
	D3D12Helper::CreateCBVAndRegistToDescriptorHeap(device, descriptorHeapBasic, constantBuffer, 0);
	D3D12Helper::CreateSRVAndRegistToDescriptorHeap(device, descriptorHeapBasic, textureBuffer, 1, texture.format);

	const auto [vertexBufferView, indexBufferView]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, mesh);

	const auto [shaderVS, shaderPS]
		= D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");

	const ViewportScissorRect viewportScissorRect
		= ViewportScissorRect::CreateFullSized(WindowWidth, WindowHeight);

	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayouts, FillMode::Solid, CullMode::None);

	BEGIN_MESSAGE_LOOP;
	{
		const auto [currentBackBuffer, currentBackBufferRTV]
			= D3D12BasicFlow::GetCurrentBackBufferAndView(device, swapChain, descriptorHeapRTV);

		// 適当に、立方体を回転させる
		transform.rotation = Quaternion::FromAxisAngle(Vector3::Up(), 1.0f * DegToRad) * transform.rotation;
		constantBufferVirtualPtr->Matrix_M_IT = transform.CalculateModelMatrixInversed().Transposed();
		constantBufferVirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(transform, cameraTransform);

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
	END_MESSAGE_LOOP;
}
END_INITIALIZE(0);
