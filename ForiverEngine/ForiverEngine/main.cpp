//#define ENABLE_CUI_CONSOLE

#include "scripts/headers/D3D12BasicFlow.h"

constexpr int WindowWidth = 1344;
constexpr int WindowHeight = 756;

BEGIN_INITIALIZE(L"ForiverEngine", L"ForiverEngine", hwnd, WindowWidth, WindowHeight);
{
	using namespace ForiverEngine;

	//Print("Hello, World!\n");

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	WindowHelper::SetTargetFps(60);
	WindowHelper::SetCursorEnabled(false);

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

	constexpr Transform terrainTransform = Transform::Identity();
	CameraTransform cameraTransform = CameraTransform::CreateBasic(
		Vector3(5, 6, 5), Quaternion::Identity(), 60.0f * DegToRad, 1.0f * WindowWidth / WindowHeight);

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
		.Matrix_M_IT = terrainTransform.CalculateModelMatrixInversed().Transposed(),
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform),
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

	bool onGround = false; // 地面に接地しているか
	float velocity = 0; // 鉛直速度
	constexpr float jumpHeight = 2.0f; // ジャンプ高さ (m)
	constexpr float eyeHeight = 1.6f; // 目の高さ (m)
	constexpr float footOffset = 0.1f; // 足元のオフセット (地面判定用)

	BEGIN_FRAME(hwnd);
	{
		// Escape でゲーム終了
		if (InputHelper::GetKeyInfo(Key::Escape).pressedNow)
			return 0;

		// 落下とジャンプ
		{
			if (onGround)
			{
				if (InputHelper::GetKeyInfo(Key::Space).pressedNow)
				{
					onGround = false;
					velocity += std::sqrt(2.0f * G * jumpHeight);
				}
			}

			Vector3 positionBeforeMoveV = cameraTransform.position;
			if (!onGround)
			{
				velocity -= G * WindowHelper::GetDeltaSeconds();
				cameraTransform.position += Vector3::Up() * (velocity * WindowHelper::GetDeltaSeconds());
			}

			if (velocity <= 0.0f)
			{
				Lattice3 footPosition = Lattice3(cameraTransform.position - Vector3::Up() * (eyeHeight + footOffset));
				if (terrainData[footPosition.y][footPosition.z][footPosition.x] != 0)
				{
					// 着地
					onGround = true;
					velocity = 0;
					cameraTransform.position = positionBeforeMoveV;
				}
			}
		}

		// キー入力でカメラを移動・回転させる
		{
			constexpr float cameraMoveHSpeed = 3.0f; // m/s
			constexpr float DashSpeedMultiplier = 2.0f; // ダッシュ時の速度倍率
			constexpr float cameraRotateHSpeed = 180.0f * DegToRad; // rad/s
			constexpr float cameraRotateVSpeed = 90.0f * DegToRad; // rad/s

			// 回転
			const Vector2 rotateInput = InputHelper::GetAsAxis2D(Key::Up, Key::Down, Key::Left, Key::Right);
			const Quaternion cameraRotateAmount =
				Quaternion::FromAxisAngle(Vector3::Up(), rotateInput.x * cameraRotateHSpeed * WindowHelper::GetDeltaSeconds()) *
				Quaternion::FromAxisAngle(cameraTransform.GetRight(), -rotateInput.y * cameraRotateVSpeed * WindowHelper::GetDeltaSeconds());
			Quaternion newRotation = cameraRotateAmount * cameraTransform.rotation;
			if (std::abs((newRotation * Vector3::Forward()).y) < 0.99f) // 上下回転の制限 (前方向ベクトルのy成分で判定. 判定は大きく)
				cameraTransform.rotation = newRotation;

			// 移動
			Vector2 cameraMoveHInput = InputHelper::GetAsAxis2D(Key::W, Key::S, Key::A, Key::D);
			Vector3 cameraMoveHDirection = cameraTransform.rotation * Vector3(cameraMoveHInput.x, 0.0f, cameraMoveHInput.y);
			cameraMoveHDirection.y = 0.0f; // 水平成分のみ
			cameraMoveHDirection.Norm(); // 最後に正規化する
			float speed = cameraMoveHSpeed;
			if (InputHelper::GetKeyInfo(Key::LShift).pressed)
				speed *= DashSpeedMultiplier;
			cameraTransform.position += cameraMoveHDirection * (speed * WindowHelper::GetDeltaSeconds());

			// これだけ再計算すれば良い
			cbvBufferVirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform);
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
			viewportScissorRect, PrimitiveTopology::TriangleList, Color::CreateFromUint8(60, 150, 210), DepthBufferClearValue,
			static_cast<int>(mesh.indices.size())
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_FRAME;
}
END_INITIALIZE(0);
