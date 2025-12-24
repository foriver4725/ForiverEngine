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
		Vector3(64, 32, 64), Quaternion::Identity(), 60.0f * DegToRad, 1.0f * WindowWidth / WindowHeight);

	// 地形データ
	constexpr int TerrainSeed = 0x2961E3B1;
	constexpr int ChunkCount = 16; // ワールドのチャンク数 (ChunkCount x ChunkCount 個)
	std::array<std::array<Terrain, ChunkCount>, ChunkCount> terrains = {}; // チャンクの配列
	std::array<std::array<Mesh, ChunkCount>, ChunkCount> terrainMeshes = {}; // 地形の結合メッシュ
	for (int chunkX = 0; chunkX < ChunkCount; ++chunkX)
		for (int chunkZ = 0; chunkZ < ChunkCount; ++chunkZ)
		{
			const Terrain terrain = Terrain::CreateFromNoise({ chunkX, chunkZ }, { 0.015f, 12.0f }, TerrainSeed, 16, 18, 24);
			terrains[chunkX][chunkZ] = terrain;

			const Lattice2 localOffset = Lattice2(chunkX * Terrain::ChunkSize, chunkZ * Terrain::ChunkSize);
			terrainMeshes[chunkX][chunkZ] = terrain.CreateMesh(localOffset);
		}

	// b0 レジスタに渡すデータ
	struct alignas(256) CBData0
	{
		Matrix4x4 Matrix_M; // M
		Matrix4x4 Matrix_M_IT; // M の逆→転置行列
		Matrix4x4 Matrix_MVP; // MVP

		Vector3 SelectingBlockPosition; // 選択中のブロック位置 (ワールド座標)
		float IsSelectingAnyBlock; // ブロックを選択中かどうか (bool 型として扱う)
		Color SelectColor; // 選択中のブロックの乗算色 (a でブレンド率を指定)
	};

	CBData0 cbData0 =
	{
		.Matrix_M = terrainTransform.CalculateModelMatrix(),
		.Matrix_M_IT = terrainTransform.CalculateModelMatrixInversed().Transposed(),
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform),

		.SelectingBlockPosition = Vector3::Zero(),
		.IsSelectingAnyBlock = 0,
		.SelectColor = Color::CreateFromUint8(255, 255, 0, 100),
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

	// 向いているブロックを選択
	constexpr float ReachDistance = 5.0f; // 選択可能な最大距離 (m)
	constexpr float ReachDetectStep = 0.1f; // レイキャストの刻み幅 (m)

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

		// ブロックを選択する
		{
			const Vector3 rayOrigin = cameraTransform.position;
			const Vector3 rayDirection = cameraTransform.GetForward().Normed();

			Block hitBlock = Block::Air;
			Lattice3 hitPosition = Lattice3::Zero();
			for (float d = 0.0f; d <= ReachDistance; d += ReachDetectStep)
			{
				const Vector3 rayPosition = rayOrigin + rayDirection * d;
				const Lattice3 rayPositionAsLattice = Lattice3(
					static_cast<int>(std::round(rayPosition.x)),
					static_cast<int>(std::round(rayPosition.y)),
					static_cast<int>(std::round(rayPosition.z))
				);

				const Lattice2 chunkIndex = PlayerControl::GetChunkIndexAtPosition(rayPosition);
				if (chunkIndex.x < 0 || chunkIndex.x >= ChunkCount
					|| chunkIndex.y < 0 || chunkIndex.y >= ChunkCount)
				{
					continue; // チャンク外
				}
				const Terrain& targetTerrain = terrains[chunkIndex.x][chunkIndex.y];
				const Lattice3 rayLocalPosition = Lattice3(
					std::clamp(rayPositionAsLattice.x - chunkIndex.x * Terrain::ChunkSize, 0, Terrain::ChunkSize - 1),
					std::clamp(rayPositionAsLattice.y, 0, Terrain::ChunkHeight - 1),
					std::clamp(rayPositionAsLattice.z - chunkIndex.y * Terrain::ChunkSize, 0, Terrain::ChunkSize - 1)
				);

				const Block blockAtRay = targetTerrain.GetBlock(rayLocalPosition);
				if (blockAtRay != Block::Air)
				{
					hitBlock = blockAtRay;
					hitPosition = rayPositionAsLattice;
					break;
				}
			}

			cbvBufferVirtualPtr->IsSelectingAnyBlock = (hitBlock != Block::Air) ? 1.0f : 0.0f;
			if (hitBlock != Block::Air)
			{
				cbvBufferVirtualPtr->SelectingBlockPosition = Vector3(
					static_cast<float>(hitPosition.x),
					static_cast<float>(hitPosition.y),
					static_cast<float>(hitPosition.z)
				);
			}
		}

		// ブロックを壊す
		{
			if (InputHelper::GetKeyInfo(Key::Enter).pressedNow)
			{
				if (cbvBufferVirtualPtr->IsSelectingAnyBlock > 0.5f)
				{
					const Lattice3 blockPositionAsLattice = Lattice3(cbvBufferVirtualPtr->SelectingBlockPosition);
					const Lattice2 chunkIndex = PlayerControl::GetChunkIndexAtPosition(cbvBufferVirtualPtr->SelectingBlockPosition);

					// 地形データとメッシュを更新
					terrains[chunkIndex.x][chunkIndex.y].SetBlock(
						Lattice3(
							blockPositionAsLattice.x - chunkIndex.x * Terrain::ChunkSize,
							blockPositionAsLattice.y,
							blockPositionAsLattice.z - chunkIndex.y * Terrain::ChunkSize
						),
						Block::Air);
					const Lattice2 localOffset = Lattice2(
						chunkIndex.x * Terrain::ChunkSize,
						chunkIndex.y * Terrain::ChunkSize);
					terrainMeshes[chunkIndex.x][chunkIndex.y] = terrains[chunkIndex.x][chunkIndex.y].CreateMesh(localOffset);

					// 頂点バッファーとインデックスバッファーを再作成
					const int index = chunkIndex.x * ChunkCount + chunkIndex.y;
					std::tie(vertexBufferViews[index], indexBufferViews[index])
						= D3D12BasicFlow::CreateVertexAndIndexBufferViews(
							device, terrainMeshes[chunkIndex.x][chunkIndex.y]);
					indexCounts[index] = static_cast<int>(terrainMeshes[chunkIndex.x][chunkIndex.y].indices.size());
				}
			}
		}

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
