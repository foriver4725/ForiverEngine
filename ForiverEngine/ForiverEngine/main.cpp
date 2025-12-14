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

	const auto [factory, device, commandAllocater, commandList, commandQueue, swapChain, descriptorHeapRTV]
		= D3D12BasicFlow::CreateStandardObjects(hwnd, WindowWidth, WindowHeight, false);

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

	const Matrix4x4 ModelMatrix = transform.CalculateModelMatrix();
	const Matrix4x4 ViewMatrix = cameraTransform.CalculateViewMatrix();
	const Matrix4x4 ProjectionMatrix = cameraTransform.CalculateProjectionMatrix();
	Matrix4x4 MVPMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// 頂点データ
	const std::vector<VertexData> vertices =
	{
		// Up
		{ Vector4(-1, 1, -1), Vector2(0.25f, 0.50f), Vector3(0, 1, 0)},
		{ Vector4(-1, 1, 1), Vector2(0.25f, 0.25f), Vector3(0, 1, 0)},
		{ Vector4(1, 1, -1), Vector2(0.50f, 0.50f), Vector3(0, 1, 0)},
		{ Vector4(1, 1, 1), Vector2(0.50f, 0.25f), Vector3(0, 1, 0)},

		// Down
		{ Vector4(-1, -1, 1), Vector2(1.00f, 0.25f), Vector3(0, -1, 0)},
		{ Vector4(-1, -1, -1), Vector2(1.00f, 0.50f), Vector3(0, -1, 0)},
		{ Vector4(1, -1, 1), Vector2(0.75f, 0.25f), Vector3(0, -1, 0)},
		{ Vector4(1, -1, -1), Vector2(0.75f, 0.50f), Vector3(0, -1, 0)},

		// Right
		{ Vector4(1, -1, -1), Vector2(0.75f, 0.50f), Vector3(1, 0, 0)},
		{ Vector4(1, 1, -1), Vector2(0.50f, 0.50f), Vector3(1, 0, 0)},
		{ Vector4(1, -1, 1), Vector2(0.75f, 0.25f), Vector3(1, 0, 0)},
		{ Vector4(1, 1, 1), Vector2(0.50f, 0.25f), Vector3(1, 0, 0)},

		// Left
		{ Vector4(-1, -1, 1), Vector2(0.00f, 0.25f), Vector3(-1, 0, 0)},
		{ Vector4(-1, 1, 1), Vector2(0.25f, 0.25f), Vector3(-1, 0, 0)},
		{ Vector4(-1, -1, -1), Vector2(0.00f, 0.50f), Vector3(-1, 0, 0)},
		{ Vector4(-1, 1, -1), Vector2(0.25f, 0.50f), Vector3(-1, 0, 0)},

		// Forward
		{ Vector4(1, -1, 1), Vector2(0.50f, 0.00f), Vector3(0, 0, 1)},
		{ Vector4(1, 1, 1), Vector2(0.50f, 0.25f), Vector3(0, 0, 1)},
		{ Vector4(-1, -1, 1), Vector2(0.25f, 0.00f), Vector3(0, 0, 1)},
		{ Vector4(-1, 1, 1), Vector2(0.25f, 0.25f), Vector3(0, 0, 1)},

		// Backward
		{ Vector4(-1, -1, -1), Vector2(0.25f, 0.75f), Vector3(0, 0, -1)},
		{ Vector4(-1, 1, -1), Vector2(0.25f, 0.50f), Vector3(0, 0, -1)},
		{ Vector4(1, -1, -1), Vector2(0.50f, 0.75f), Vector3(0, 0, -1)},
		{ Vector4(1, 1, -1), Vector2(0.50f, 0.50f), Vector3(0, 0, -1)},
	};

	// 頂点インデックス (時計回り)
	const std::vector<std::uint16_t> indices =
	{
		0, 1, 2, 2, 1, 3,       // Up
		4, 5, 6, 6, 5, 7,       // Down
		8, 9, 10, 10, 9, 11,    // Right
		12, 13, 14, 14, 13, 15, // Left
		16, 17, 18, 18, 17, 19, // Forward
		20, 21, 22, 22, 21, 23, // Backward
	};

	// 頂点レイアウト
	const std::vector<VertexLayout> vertexLayouts =
	{
		{ "POSITION", Format::RGBA_F32 },
		{ "TEXCOORD", Format::RG_F32 },
		{ "NORMAL", Format::RGB_F32 },
	};

	// 定数バッファー (サイズは256バイトにアラインメントする必要がある!!)
	const GraphicsBuffer constantBuffer = D3D12Helper::CreateGraphicsBuffer1D(device, GetAlignmentedSize(sizeof(MVPMatrix), 256), true);
	if (!constantBuffer)
		ShowError(L"定数バッファーの作成に失敗しました");
	// 定数バッファーにデータを書き込む
	// Unmap しないでおく
	Matrix4x4* constantBufferVirtualPtr = nullptr;
	if (!D3D12Helper::CopyDataFromCPUToGPUThroughGraphicsBuffer1D(constantBuffer, &MVPMatrix, sizeof(MVPMatrix),
		false, reinterpret_cast<void**>(&constantBufferVirtualPtr)))
		ShowError(L"定数バッファーへのデータ転送に失敗しました");

	// 立方体を切り開いた展開図
	// Up, Down, Right, Left, Forward, Backward
	//    [F]
	// [L][U][R][D]
	//    [B]
	const Texture texture = AssetLoader::LoadTexture("assets/textures/grass.png");
	if (!texture.IsValid())
		ShowError(L"テクスチャのロードに失敗しました");
	const GraphicsBuffer textureCopyIntermediateBuffer = D3D12Helper::CreateGraphicsBuffer1D(device,
		static_cast<int>(GetAlignmentedSize(texture.rowSize, Texture::RowSizeAlignment) * texture.height), true);
	if (!textureCopyIntermediateBuffer)
		ShowError(L"テクスチャ転送用中間バッファの作成に失敗しました");
	const GraphicsBuffer textureBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, texture);
	if (!textureBuffer)
		ShowError(L"テクスチャバッファの作成に失敗しました");

	if (!D3D12Helper::CommandCopyDataFromCPUToGPUThroughGraphicsBufferTexture2D(commandList, textureCopyIntermediateBuffer, textureBuffer, texture))
		ShowError(L"テクスチャデータのアップロードに失敗しました");
	D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, textureBuffer,
		GraphicsBufferState::CopyDestination, GraphicsBufferState::PixelShaderResource, false);
	D3D12Helper::CommandClose(commandList);
	D3D12Helper::ExecuteCommands(commandQueue, commandList);
	if (!D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue))
		ShowError(L"GPU の処理待ち受けに失敗しました");

	// CBV, SRV から成る DescriptorHeap
	const DescriptorHeap descriptorHeap = D3D12Helper::CreateDescriptorHeap(device, DescriptorHeapType::CBV_SRV_UAV, 2, true);
	if (!descriptorHeap)
		ShowError(L"CBV/SRV/UAV 用 DescriptorHeap の作成に失敗しました");
	D3D12Helper::CreateCBVAndRegistToDescriptorHeap(device, descriptorHeap, constantBuffer, 0);
	D3D12Helper::CreateSRVAndRegistToDescriptorHeap(device, descriptorHeap, textureBuffer, 1, texture.format);

	const auto [vertexBufferView, indexBufferView]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, vertices, indices);

	const auto [shaderVS, shaderPS]
		= D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");

	const ViewportScissorRect viewportScissorRect
		= ViewportScissorRect::CreateFullSized(WindowWidth, WindowHeight);

	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, vertexLayouts, FillMode::Solid, CullMode::None);

	BEGIN_MESSAGE_LOOP;
	{
		const auto [currentBackBuffer, currentBackBufferRTV]
			= D3D12BasicFlow::GetCurrentBackBufferAndCreateView(device, swapChain, descriptorHeapRTV);

		// 適当に、立方体を回転させる
		transform.rotation = Quaternion::FromAxisAngle(Vector3::Up(), 1.0f * DegToRad) * transform.rotation;
		const Matrix4x4 ModelMatrix = transform.CalculateModelMatrix();
		const Matrix4x4 ViewMatrix = cameraTransform.CalculateViewMatrix();
		const Matrix4x4 ProjectionMatrix = cameraTransform.CalculateProjectionMatrix();
		*constantBufferVirtualPtr = ProjectionMatrix * ViewMatrix * ModelMatrix;

		D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, currentBackBuffer,
			GraphicsBufferState::Present, GraphicsBufferState::RenderTarget, false);
		{
			D3D12Helper::CommandSetRTAsOutputStage(commandList, currentBackBufferRTV);
			D3D12Helper::CommandClearRT(commandList, currentBackBufferRTV, Color::Black());
			D3D12Helper::CommandSetRootSignature(commandList, rootSignature);
			D3D12Helper::CommandSetGraphicsPipelineState(commandList, graphicsPipelineState);
			D3D12Helper::CommandSetDescriptorHeaps(commandList, { descriptorHeap });
			D3D12Helper::CommandLinkRootParameterIndexAndDescriptorHeapHandleAtGPU(
				commandList, device, descriptorHeap, DescriptorHeapType::CBV_SRV_UAV, 0, 0); // ルートパラメーターは1つだけ
			D3D12Helper::CommandIASetPrimitiveTopology(commandList, PrimitiveTopology::TriangleList);
			D3D12Helper::CommandIASetVertexBuffer(commandList, { vertexBufferView });
			D3D12Helper::CommandIASetIndexBuffer(commandList, indexBufferView);
			D3D12Helper::CommandRSSetViewportAndScissorRect(commandList, viewportScissorRect);
			D3D12Helper::CommandDrawIndexedInstanced(commandList, static_cast<int>(indices.size()));
		}
		D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, currentBackBuffer,
			GraphicsBufferState::RenderTarget, GraphicsBufferState::Present, false);
		D3D12Helper::CommandClose(commandList);
		D3D12Helper::ExecuteCommands(commandQueue, commandList);

		if (!D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue))
			ShowError(L"GPU の処理待ち受けに失敗しました");

		if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocater, commandList))
			ShowError(L"CommandAllocator, CommandList のクリアに失敗しました");

		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_MESSAGE_LOOP;
}
END_INITIALIZE(0);
