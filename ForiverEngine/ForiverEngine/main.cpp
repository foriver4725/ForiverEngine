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

	// 定数バッファー (サイズは256バイトにアラインメントする必要がある!!)
	const GraphicsBuffer constantBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, GetAlignmentedSize(sizeof(cbData0), 256), true);
	if (!constantBuffer)
		ShowError(L"定数バッファーの作成に失敗しました");
	// 定数バッファーにデータを書き込む
	// Unmap しないでおく
	CBData0* cbData0VirtualPtr = nullptr;
	if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(constantBuffer, &cbData0, sizeof(cbData0),
		false, reinterpret_cast<void**>(&cbData0VirtualPtr)))
		ShowError(L"定数バッファーへのデータ転送に失敗しました");

	const Texture textureArray = AssetLoader::LoadTextureArray(
		{
			"assets/textures/grass_stone.png",
			"assets/textures/dirt_sand.png",
		});
	if (!textureArray.IsValid())
		ShowError(L"テクスチャ群のロードに失敗しました");
	const GraphicsBuffer textureArrayBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, textureArray);
	if (!textureArrayBuffer)
		ShowError(L"テクスチャ配列バッファの作成に失敗しました");
	D3D12BasicFlow::UploadTextureToGPU(commandList, commandQueue, device, textureArrayBuffer, textureArray);

	// CBV, SRV から成る DescriptorHeap
	const DescriptorHeap descriptorHeapBasic = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::CBV_SRV_UAV, 2, true);
	if (!descriptorHeapBasic)
		ShowError(L"CBV/SRV/UAV 用 DescriptorHeap の作成に失敗しました");
	D3D12Helper::CreateCBVAndRegistToDescriptorHeap(device, descriptorHeapBasic, constantBuffer, 0);
	D3D12Helper::CreateSRVAndRegistToDescriptorHeap(device, descriptorHeapBasic, textureArrayBuffer, 1, textureArray);

	const auto [vertexBufferView, indexBufferView]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, mesh);

	const ViewportScissorRect viewportScissorRect
		= ViewportScissorRect::CreateFullSized(WindowWidth, WindowHeight);

	BEGIN_MESSAGE_LOOP;
	{
		// 適当に、立方体を回転させる
#if false
		transform.rotation = Quaternion::FromAxisAngle(Vector3::Up(), 1.0f * DegToRad) * transform.rotation;
		cbData0VirtualPtr->Matrix_M_IT = transform.CalculateModelMatrixInversed().Transposed();
		cbData0VirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(transform, cameraTransform);
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
	END_MESSAGE_LOOP;
}
END_INITIALIZE(0);
