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
	const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVS, shaderPS] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");
	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayouts, FillMode::Solid, CullMode::None);

	const auto [rtBufferGetter, rtvGetter] = D3D12BasicFlow::InitRTV(device, swapChain, 2, false);
	const DescriptorHeapHandleAtCPU dsv = D3D12BasicFlow::InitDSV(device, WindowWidth, WindowHeight, DepthBufferClearValue);

	// ボクセルなので、この値がデフォルト値から変化することはないはず
	Transform transform =
	{
		.parent = nullptr,
		.position = Vector3::Zero(),
		.rotation = Quaternion::Identity(),
		.scale = Vector3::One(),
	};

	CameraTransform cameraTransform = CameraTransform::CreateBasic(
		Vector3(5, 4, 5), Quaternion::Identity(), 60.0f * DegToRad, 1.0f * WindowWidth / WindowHeight);

	// 地形データ
	std::vector<std::vector<std::vector<std::uint32_t>>> terrainData;
	{
		// 取り合えず 16x16x16 の空間
		terrainData =
			std::vector<std::vector<std::vector<std::uint32_t>>>(
				16,
				std::vector<std::vector<std::uint32_t>>(
					16,
					std::vector<std::uint32_t>(
						16,
						0 // 初期値は空気
					)
				)
			);

		// y=3 以下が地面
		for (int y = 0; y <= 3; ++y)
		{
			for (int z = 0; z < 16; ++z)
			{
				for (int x = 0; x < 16; ++x)
				{
					terrainData[y][z][x] = 2; // 取り合えず草で埋める
				}
			}
		}

		// (15,15) だけ砂にする
		terrainData[3][15][15] = 5;
	}
	const Mesh mesh = Mesh::CreateFromTerrainData(terrainData);

	// b0 レジスタに渡すデータ
	struct alignas(256) CBData0
	{
		Matrix4x4 Matrix_M_IT; // M の逆→転置行列
		Matrix4x4 Matrix_MVP; // MVP
	};

	CBData0 cbData0 =
	{
		.Matrix_M_IT = transform.CalculateModelMatrixInversed().Transposed(),
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(transform, cameraTransform),
	};

	// CBV 用バッファ
	CBData0* cbvBufferVirtualPtr = nullptr;
	const GraphicsBuffer cbvBuffer = D3D12BasicFlow::InitCBVBuffer<CBData0>(device, cbData0, false, &cbvBufferVirtualPtr);

	// SRV 用バッファ
	const auto srvBufferAndData = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue,
		{
			"assets/textures/air_invalid.png",
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
		// キー入力でカメラを移動・回転させる
		{
			constexpr float cameraMoveSpeed = 3.0f; // m/s
			constexpr float cameraMoveVSpeed = 2.0f; // m/s (上下方向)
			constexpr float cameraRotateSpeed = 180.0f * DegToRad; // rad/s

			// 回転
			Quaternion cameraRotateAmount = Quaternion::Identity();
			if (InputHelper::GetKeyInfo(Key::Right).pressed)
				cameraRotateAmount = Quaternion::FromAxisAngle(Vector3::Up(), cameraRotateSpeed * WindowHelper::GetDeltaSeconds()) * cameraRotateAmount;
			if (InputHelper::GetKeyInfo(Key::Left).pressed)
				cameraRotateAmount = Quaternion::FromAxisAngle(Vector3::Up(), -cameraRotateSpeed * WindowHelper::GetDeltaSeconds()) * cameraRotateAmount;
			cameraTransform.rotation = cameraRotateAmount * cameraTransform.rotation;

			// 移動
			Vector3 cameraMoveDirection = Vector3::Zero(); // ローカル座標系
			Vector3 cameraMoveVAmount = Vector3::Zero(); // 上下方向の移動 (別個に計算)
			if (InputHelper::GetKeyInfo(Key::W).pressed)
				cameraMoveDirection += Vector3::Forward();
			if (InputHelper::GetKeyInfo(Key::S).pressed)
				cameraMoveDirection += Vector3::Backward();
			if (InputHelper::GetKeyInfo(Key::D).pressed)
				cameraMoveDirection += Vector3::Right();
			if (InputHelper::GetKeyInfo(Key::A).pressed)
				cameraMoveDirection += Vector3::Left();
			cameraMoveDirection.Norm();
			Vector3 cameraMoveDirectionWorld = cameraTransform.rotation * cameraMoveDirection; // ワールド座標系に変換 (yは0のはず)

			if (InputHelper::GetKeyInfo(Key::Space).pressed)
				cameraMoveVAmount += Vector3::Up() * (cameraMoveVSpeed * WindowHelper::GetDeltaSeconds());
			if (InputHelper::GetKeyInfo(Key::LShift).pressed)
				cameraMoveVAmount += Vector3::Down() * (cameraMoveVSpeed * WindowHelper::GetDeltaSeconds());

			cameraTransform.position += cameraMoveDirectionWorld * (cameraMoveSpeed * WindowHelper::GetDeltaSeconds()) + cameraMoveVAmount;

			// これだけ再計算すれば良い
			cbvBufferVirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(transform, cameraTransform);
		}

		const int currentBackBufferIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);
		const GraphicsBuffer currentBackBuffer = rtBufferGetter(currentBackBufferIndex);
		const DescriptorHeapHandleAtCPU currentBackBufferRTV = rtvGetter(currentBackBufferIndex);
		if (!currentBackBuffer)
			ShowError(L"現在のバックバッファーの取得に失敗しました");

		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignature, graphicsPipelineState, currentBackBuffer,
			currentBackBufferRTV, dsv, descriptorHeapBasic, { vertexBufferView }, indexBufferView,
			viewportScissorRect, PrimitiveTopology::TriangleList, Color::Black(), DepthBufferClearValue,
			static_cast<int>(mesh.indices.size())
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_FRAME;
}
END_INITIALIZE(0);
