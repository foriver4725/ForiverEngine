//#define ENABLE_CUI_CONSOLE

#include "scripts/headers/D3D12BasicFlow.h"
#include "scripts/headers/Terrain.h"
#include "scripts/headers/PlayerControl.h"

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
		Vector3(12, 32, 12), Quaternion::Identity(), 60.0f * DegToRad, 1.0f * WindowWidth / WindowHeight);

	// 地形データ
	constexpr int TerrainSeed = 0x2961E3B1;
	constexpr int ChunkCount = 16; // ワールドのチャンク数 (ChunkCount x ChunkCount 個)
	std::array<std::array<Terrain, ChunkCount>, ChunkCount> terrains = {}; // チャンクの配列
	std::array<std::array<Mesh, ChunkCount>, ChunkCount> terrainMeshes = {}; // 地形の結合メッシュ
	for (int chunkX = 0; chunkX < ChunkCount; ++chunkX)
		for (int chunkZ = 0; chunkZ < ChunkCount; ++chunkZ)
		{
			const Terrain terrain = Terrain::CreateFromNoise({ chunkX, chunkZ }, { 0.02f, 12.0f }, TerrainSeed, 16, 18, 24);
			terrains[chunkX][chunkZ] = terrain;

			const Lattice2 localOffset = Lattice2(chunkX * Terrain::ChunkSize, chunkZ * Terrain::ChunkSize);
			terrainMeshes[chunkX][chunkZ] = terrain.CreateMesh(localOffset);
		}

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

	std::array<VertexBufferView, (ChunkCount* ChunkCount)> vertexBufferViews = {};
	std::array<IndexBufferView, (ChunkCount* ChunkCount)> indexBufferViews = {};
	std::array<int, (ChunkCount* ChunkCount)> indexCounts = {};
	for (int chunkX = 0; chunkX < ChunkCount; ++chunkX)
		for (int chunkZ = 0; chunkZ < ChunkCount; ++chunkZ)
		{
			const int index = chunkX * ChunkCount + chunkZ;

			std::tie(vertexBufferViews[index], indexBufferViews[index])
				= D3D12BasicFlow::CreateVertexAndIndexBufferViews(
					device, terrainMeshes[chunkX][chunkZ]);

			indexCounts[index] = static_cast<int>(terrainMeshes[chunkX][chunkZ].indices.size());
		}

	const ViewportScissorRect viewportScissorRect
		= ViewportScissorRect::CreateFullSized(WindowWidth, WindowHeight);

	constexpr Vector3 PlayerCollisionSize = Vector3(0.5f, 1.8f, 0.5f);
	constexpr float SpeedH = 3.0f; // 水平移動速度 (m/s)
	constexpr float DashSpeedH = 6.0f; // ダッシュ時の水平移動速度 (m/s)
	constexpr float CameraSensitivityH = 180.0f; // 水平感度 (度/s)
	constexpr float CameraSensitivityV = 90.0f; // 垂直感度 (度/s)
	constexpr float MinVelocityV = -50.0f; // 最大落下速度 (m/s)
	constexpr float JumpHeight = 1.1f; // ジャンプ高さ (m)
	constexpr float EyeHeight = 1.6f; // 目の高さ (m)
	constexpr float GroundedCheckOffset = 0.1f; // 接地判定のオフセット (m). 埋まっている判定と区別するため、少しずらす
	float velocityV = 0; // 鉛直速度

	BEGIN_FRAME(hwnd);
	{
		// Escape でゲーム終了
		if (InputHelper::GetKeyInfo(Key::Escape).pressedNow)
			return 0;

		// 回転
		PlayerControl::Rotate(
			cameraTransform,
			InputHelper::GetAsAxis2D(Key::Up, Key::Down, Key::Left, Key::Right),
			Vector2(CameraSensitivityH, CameraSensitivityV) * DegToRad,
			WindowHelper::GetDeltaSeconds()
		);

		// 移動前の座標を保存しておく
		const Vector3 positionBeforeMove = cameraTransform.position;

		// 落下とジャンプ
		{
			// 設置判定
			const int surfaceY = PlayerControl::GetFootSurfaceHeight(
				terrains,
				PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight),
				PlayerCollisionSize);
			const bool isGrounded = surfaceY > 0 ? (cameraTransform.position.y - EyeHeight <= surfaceY + 0.5f + GroundedCheckOffset) : false;

			if (isGrounded)
			{
				if (velocityV < 0)
					velocityV = 0;

				// 地面へのめり込みを補正する
				const float standY = surfaceY + 0.5f + EyeHeight;
				if (cameraTransform.position.y < standY)
					cameraTransform.position.y = standY;

				if (InputHelper::GetKeyInfo(Key::Space).pressedNow)
					velocityV = std::sqrt(2.0f * G * JumpHeight);
			}
			else
			{
				velocityV -= G * WindowHelper::GetDeltaSeconds();
				velocityV = std::max(velocityV, MinVelocityV);
			}

			if (std::abs(velocityV) > 0.01f)
				cameraTransform.position += Vector3::Up() * (velocityV * WindowHelper::GetDeltaSeconds());
		}

		// 移動
		{
			const Vector2 moveInput = InputHelper::GetAsAxis2D(Key::W, Key::S, Key::A, Key::D);
			const bool canDash = moveInput.y > 0.5f; // 前進しているときのみダッシュ可能

			PlayerControl::MoveH(
				cameraTransform,
				moveInput,
				(canDash && InputHelper::GetKeyInfo(Key::LShift).pressed) ? DashSpeedH : SpeedH,
				WindowHelper::GetDeltaSeconds()
			);
		}

		// 当たり判定 (主に水平)
		if (PlayerControl::IsOverlappingWithTerrain(
			terrains,
			PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight),
			PlayerCollisionSize))
		{
			// 衝突していたら移動前の位置に戻す
			// XZ 平面のみ戻す
			cameraTransform.position = Vector3(positionBeforeMove.x, cameraTransform.position.y, positionBeforeMove.z);
		}

		// これだけ再計算すれば良い
		cbvBufferVirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform);

		const int currentBackBufferIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);
		const GraphicsBuffer currentBackBuffer = rtBufferGetter(currentBackBufferIndex);
		const DescriptorHeapHandleAtCPU currentBackBufferRTV = rtvGetter(currentBackBufferIndex);
		if (!currentBackBuffer)
			ShowError(L"現在のバックバッファーの取得に失敗しました");

		D3D12BasicFlow::CommandBasicLoop<ChunkCount* ChunkCount>(
			commandList, commandQueue, commandAllocator, device,
			rootSignature, graphicsPipelineState, currentBackBuffer,
			currentBackBufferRTV, dsv, descriptorHeapBasic, vertexBufferViews, indexBufferViews,
			viewportScissorRect, PrimitiveTopology::TriangleList, Color::CreateFromUint8(60, 150, 210), DepthBufferClearValue,
			indexCounts
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_FRAME;
}
END_INITIALIZE(0);
