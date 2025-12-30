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

	constexpr Color RTClearColor = Color::CreateFromUint8(60, 150, 210); // 空色

	const auto [factory, device, commandAllocator, commandList, commandQueue, swapChain]
		= D3D12BasicFlow::CreateStandardObjects(hwnd, WindowWidth, WindowHeight);

	const RootParameter rootParameter = RootParameter::CreateBasic(1, 1, 0);
	const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVS, shaderPS] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");
	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayouts, FillMode::Solid, CullMode::None, true);

	const auto [rtGetter, rtvGetter] = D3D12BasicFlow::InitRTV(device, swapChain, 2, false);
	const DescriptorHeapHandleAtCPU dsv = D3D12BasicFlow::InitDSV(device, WindowWidth, WindowHeight, DepthBufferClearValue);

	constexpr Transform terrainTransform = Transform::Identity();
	CameraTransform cameraTransform = CameraTransform::CreateBasic(
		Vector3(64, 32, 64), Quaternion::Identity(), 60.0f * DegToRad, 1.0f * WindowWidth / WindowHeight);

	// 地形データ
	constexpr int TerrainSeed = 0x2961E3B1;
	constexpr int ChunkCount = 32; // ワールドのチャンク数 (ChunkCount x ChunkCount 個)
	std::array<std::array<Terrain, ChunkCount>, ChunkCount> terrains = {}; // チャンクの配列
	std::array<std::array<Mesh, ChunkCount>, ChunkCount> terrainMeshes = {}; // 地形の結合メッシュ
	std::array<std::array<VertexBufferView, ChunkCount>, ChunkCount> terrainVertexBufferViews = {}; // 頂点バッファビュー (全部)
	std::array<std::array<IndexBufferView, ChunkCount>, ChunkCount> terrainIndexBufferViews = {}; // インデックスバッファビュー (全部)
	for (int chunkX = 0; chunkX < ChunkCount; ++chunkX)
		for (int chunkZ = 0; chunkZ < ChunkCount; ++chunkZ)
		{
			const Terrain terrain = Terrain::CreateFromNoise({ chunkX, chunkZ }, { 0.015f, 12.0f }, TerrainSeed, 16, 18, 24);
			terrains[chunkX][chunkZ] = terrain;

			const Lattice2 localOffset = Lattice2(chunkX * Terrain::ChunkSize, chunkZ * Terrain::ChunkSize);
			terrainMeshes[chunkX][chunkZ] = terrain.CreateMesh(localOffset);

			const auto [vertexBufferView, indexBufferView]
				= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, terrainMeshes[chunkX][chunkZ]);
			terrainVertexBufferViews[chunkX][chunkZ] = vertexBufferView;
			terrainIndexBufferViews[chunkX][chunkZ] = indexBufferView;
		}
	constexpr int ChunkDrawDistance = 8; // 描画チャンク数 (プレイヤーを中心に 半径 ChunkDrawDistance の矩形内のチャンクのみ描画する)
	constexpr int ChunkDrawMaxCount = (ChunkDrawDistance * 2 + 1) * (ChunkDrawDistance * 2 + 1);
	// 描画するチャンクが変化した時、ビューを再作成する
	Lattice2 chunkIndex = PlayerControl::GetChunkIndexAtPosition(cameraTransform.position);
	int chunkDrawIndexXMin = std::max(0, chunkIndex.x - ChunkDrawDistance);
	int chunkDrawIndexXMax = std::min(ChunkCount - 1, chunkIndex.x + ChunkDrawDistance);
	int chunkDrawIndexZMin = std::max(0, chunkIndex.y - ChunkDrawDistance);
	int chunkDrawIndexZMax = std::min(ChunkCount - 1, chunkIndex.y + ChunkDrawDistance);
	// 一応計算しておく (<= chunkDrawMaxCount)
	int chunkDrawCount = (chunkDrawIndexXMax - chunkDrawIndexXMin + 1) * (chunkDrawIndexZMax - chunkDrawIndexZMin + 1);
	// 描画するビューとインデックス (初回作成)
	std::vector<VertexBufferView> drawingVertexBufferViews; drawingVertexBufferViews.reserve(ChunkDrawMaxCount);
	std::vector<IndexBufferView> drawingIndexBufferViews; drawingIndexBufferViews.reserve(ChunkDrawMaxCount);
	std::vector<int> drawingIndexCounts; drawingIndexCounts.reserve(ChunkDrawMaxCount);
	{
		for (int chunkX = chunkDrawIndexXMin; chunkX <= chunkDrawIndexXMax; ++chunkX)
			for (int chunkZ = chunkDrawIndexZMin; chunkZ <= chunkDrawIndexZMax; ++chunkZ)
			{
				const auto [vertexBufferView, indexBufferView]
					= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, terrainMeshes[chunkX][chunkZ]);
				const int indexCount = static_cast<int>(terrainMeshes[chunkX][chunkZ].indices.size());

				drawingVertexBufferViews.push_back(vertexBufferView);
				drawingIndexBufferViews.push_back(indexBufferView);
				drawingIndexCounts.push_back(indexCount);
			}
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

		Vector3 DirectionalLightDirection; // 太陽光の向き (正規化済み)
		float Pad0;
		Color DirectionalLightColor; // 太陽光の色 (a は使わない)
		Color AmbientLightColor; // 環境光の色 (a は使わない)
	};

	CBData0 cbData0 =
	{
		.Matrix_M = terrainTransform.CalculateModelMatrix(),
		.Matrix_M_IT = terrainTransform.CalculateModelMatrixInversed().Transposed(),
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform),

		.SelectingBlockPosition = Vector3::Zero(),
		.IsSelectingAnyBlock = 0,
		.SelectColor = Color::CreateFromUint8(255, 255, 0, 100),

		.DirectionalLightDirection = Vector3(1.0f, -1.0f, 1.0f).Normed(),
		.DirectionalLightColor = Color::White() * 1.2f,
		.AmbientLightColor = Color::White() * 0.5f,
	};

	// CBV 用バッファ
	CBData0* cbvBufferVirtualPtr = nullptr;
	const GraphicsBuffer cbvBuffer = D3D12BasicFlow::InitCBVBuffer<CBData0>(device, cbData0, false, &cbvBufferVirtualPtr);

	// SRV 用バッファ
	const auto srvBufferAndData = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator,
		{
			"assets/textures/air_invalid.png",
			"assets/textures/grass_stone.png",
			"assets/textures/dirt_sand.png",
		});
	const Texture textureArrayMetadata = std::get<1>(srvBufferAndData);

	// DescriptorHeap に登録
	const DescriptorHeap descriptorHeapBasic
		= D3D12BasicFlow::InitDescriptorHeapBasic(device, { cbvBuffer }, { srvBufferAndData });

	const ViewportScissorRect viewportScissorRect
		= ViewportScissorRect::CreateFullSized(WindowWidth, WindowHeight);

	//////////////////////////////
	// ポストプロセス

	// RT, SR で切り替えて使う 2D テクスチャ
	const GraphicsBuffer ppGraphicsBuffer = D3D12Helper::CreateGraphicsBufferTexture2DForRTAndSR(device, WindowWidth, WindowHeight, RTClearColor);
	const Texture ppTextureMetadata = Texture
	{
		.textureType = GraphicsBufferType::Texture2D,
		.format = Format::RGBA_U8_01, // RT と同じ. sRGB 不可.
		.width = WindowWidth,
		.height = WindowHeight,
		.rowSize = WindowWidth * 4, // RGBA なので...
		.sliceSize = WindowWidth * WindowHeight * 4,
		.sliceCount = 1,
		.mipLevels = 1,
	};



	const RootParameter rootParameterPP = RootParameter::CreateBasic(1, 1, 0);
	const SamplerConfig samplerConfigPP = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVSPP, shaderPSPP] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/PP.hlsl");
	const auto [rootSignaturePP, graphicsPipelineStatePP]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameterPP, samplerConfigPP, shaderVSPP, shaderPSPP, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, false);

	// RTVのみ作成
	const DescriptorHeapHandleAtCPU rtvPP = D3D12BasicFlow::InitRTV(device, ppGraphicsBuffer);
	const DescriptorHeapHandleAtCPU dsvPP_Dummy = DescriptorHeapHandleAtCPU{ .ptr = NULL };

	// 板ポリのメッシュ
	const MeshQuad meshPP = MeshQuad::CreateFullSized();

	// 頂点バッファビューとインデックスバッファビュー
	const auto [vertexBufferViewPP, indexBufferViewPP]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, meshPP);

	// CB 0
	struct alignas(256) CBData0PP
	{
		std::uint32_t WindowWidth;
		std::uint32_t WindowHeight;
		float LimitLuminance; // ピクセルがモデルの端にあると判断する輝度差の閾値 ([0.0, 1.0]. 小さいほどAAが多くかかる)
		float AAPower; // アンチエイリアスの強さ (大きいほどAAが強くかかる)
	};
	CBData0PP cbData0PP =
	{
		.WindowWidth = WindowWidth,
		.WindowHeight = WindowHeight,
		.LimitLuminance = 0.5f,
		.AAPower = 8.0f,
	};
	const GraphicsBuffer cbvBufferPP = D3D12BasicFlow::InitCBVBuffer<CBData0PP>(device, cbData0PP);

	// DescriptorHeap
	const DescriptorHeap descriptorHeapBasicPP
		= D3D12BasicFlow::InitDescriptorHeapBasic(device, { cbvBufferPP }, { {ppGraphicsBuffer, ppTextureMetadata} });

	//////////////////////////////

	//////////////////////////////
	// テキスト描画 (ポストプロセスの後)
	// 1文字の幅 16x16 px

	// RT, SR で切り替えて使う 2D テクスチャ
	const GraphicsBuffer textGraphicsBuffer = D3D12Helper::CreateGraphicsBufferTexture2DForRTAndSR(device, WindowWidth, WindowHeight, RTClearColor);
	const Texture textTextureMetadata = Texture
	{
		.textureType = GraphicsBufferType::Texture2D,
		.format = Format::RGBA_U8_01, // RT と同じ. sRGB 不可.
		.width = WindowWidth,
		.height = WindowHeight,
		.rowSize = WindowWidth * 4, // RGBA なので...
		.sliceSize = WindowWidth * WindowHeight * 4,
		.sliceCount = 1,
		.mipLevels = 1,
	};

	// 頂点レイアウト
	const std::vector<VertexLayout> VertexLayoutsText =
	{
		{ "POSITION" , Format::RGBA_F32 },
		{ "TEXCOORD" , Format::RG_F32   },
	};

	const RootParameter rootParameterText = RootParameter::CreateBasic(1, 2, 0);
	const SamplerConfig samplerConfigText = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVSText, shaderPSText] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/Text.hlsl");
	const auto [rootSignatureText, graphicsPipelineStateText]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameterText, samplerConfigText, shaderVSText, shaderPSText, VertexLayoutsText, FillMode::Solid, CullMode::Back, false);

	// RTVのみ作成
	const DescriptorHeapHandleAtCPU rtvText = D3D12BasicFlow::InitRTV(device, textGraphicsBuffer);
	const DescriptorHeapHandleAtCPU dsvText_Dummy = DescriptorHeapHandleAtCPU{ .ptr = NULL };

	// 板ポリのメッシュ
	const MeshQuad meshText = MeshQuad::CreateFullSized();

	// 頂点バッファビューとインデックスバッファビュー
	const auto [vertexBufferViewText, indexBufferViewText]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, meshText);

	// b0
	struct alignas(256) CBData0Text
	{
	};
	CBData0Text cbData0Text =
	{
	};
	const GraphicsBuffer cbvBufferText = D3D12BasicFlow::InitCBVBuffer<CBData0Text>(device, cbData0Text);

	// t1
	const auto fontTextureBufferAndData = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator, { "assets/font.png" });

	// DescriptorHeap
	const DescriptorHeap descriptorHeapBasicText
		= D3D12BasicFlow::InitDescriptorHeapBasic(device, { cbvBufferText }, { {textGraphicsBuffer, textTextureMetadata}, fontTextureBufferAndData });

	//////////////////////////////

	constexpr Vector3 PlayerCollisionSize = Vector3(0.5f, 1.8f, 0.5f);
	constexpr float SpeedH = 3.0f; // 水平移動速度 (m/s)
	constexpr float DashSpeedH = 6.0f; // ダッシュ時の水平移動速度 (m/s)
	constexpr float CameraSensitivityH = 180.0f; // 水平感度 (度/s)
	constexpr float CameraSensitivityV = 90.0f; // 垂直感度 (度/s)
	constexpr float MinVelocityV = -50.0f; // 最大落下速度 (m/s)
	constexpr float JumpHeight = 1.05f; // ジャンプ高さ (m)
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
			const Vector3 playerFootPosition = PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight);
			const int floorY = PlayerControl::GetFloorHeight(terrains, playerFootPosition, PlayerCollisionSize);
			const bool isGrounded = (floorY >= 0) ? (cameraTransform.position.y - EyeHeight <= floorY + 0.5f + GroundedCheckOffset) : false;

			if (isGrounded)
			{
				if (velocityV < 0)
					velocityV = 0;

				// 地面へのめり込みを補正する
				const float standY = floorY + 0.5f + EyeHeight;
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

		// 頭上にブロックがあって頭を打ったら、上方向の速度を無くす
		{
			const int ceilY = PlayerControl::GetCeilHeight(terrains, cameraTransform.position, PlayerCollisionSize);
			const bool isOverlappingCeil = (ceilY <= Terrain::ChunkHeight - 1) ? (cameraTransform.position.y - EyeHeight + PlayerCollisionSize.y >= ceilY - 0.5f) : false;

			if (isOverlappingCeil)
			{
				// 上方向の速度を無くす
				if (velocityV > 0)
					velocityV = 0;

				// 天井へのめり込みを補正する
				const float headY = ceilY - 0.5f + EyeHeight - PlayerCollisionSize.y;
				if (cameraTransform.position.y > headY)
					cameraTransform.position.y = headY;
			}
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
		// この場合、描画チャンクを必ず更新する
		bool hasBrokenBlock = false;
		{
			if (InputHelper::GetKeyInfo(Key::Enter).pressedNow)
			{
				if (cbvBufferVirtualPtr->IsSelectingAnyBlock > 0.5f)
				{
					hasBrokenBlock = true;

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

					// 頂点バッファビューとインデックスバッファビューを更新
					const auto [vertexBufferView, indexBufferView]
						= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, terrainMeshes[chunkIndex.x][chunkIndex.y]);
					terrainVertexBufferViews[chunkIndex.x][chunkIndex.y] = vertexBufferView;
					terrainIndexBufferViews[chunkIndex.x][chunkIndex.y] = indexBufferView;
				}
			}
		}

		// 描画距離内のチャンクのみ、描画する配列に追加
		// プレイヤーの存在するチャンクが変化した、またはブロックを壊した場合に更新する
		{
			// 存在するチャンクが変化したかチェック
			const Lattice2 currentChunkIndex = PlayerControl::GetChunkIndexAtPosition(cameraTransform.position);
			const bool hasExistingChunkChanged = (currentChunkIndex != chunkIndex);

			// 結局配列を更新する必要があるのか?
			const bool shouldUpdateDrawChunks = hasExistingChunkChanged || hasBrokenBlock;

			if (shouldUpdateDrawChunks)
			{
				// パラメータを更新
				chunkIndex = currentChunkIndex;
				chunkDrawIndexXMin = std::max(0, chunkIndex.x - ChunkDrawDistance);
				chunkDrawIndexXMax = std::min(ChunkCount - 1, chunkIndex.x + ChunkDrawDistance);
				chunkDrawIndexZMin = std::max(0, chunkIndex.y - ChunkDrawDistance);
				chunkDrawIndexZMax = std::min(ChunkCount - 1, chunkIndex.y + ChunkDrawDistance);
				// この後配列を更新するので、サイズが変わったかどうか保存しておく
				const int currentChunkDrawCount = (chunkDrawIndexXMax - chunkDrawIndexXMin + 1) * (chunkDrawIndexZMax - chunkDrawIndexZMin + 1);
				const int chunkDrawCountDiff = currentChunkDrawCount - chunkDrawCount;
				chunkDrawCount = currentChunkDrawCount;

				// ビューとインデックスを再作成

				// 配列のサイズ数が減ったなら、その分を削除しておく
				if (chunkDrawCountDiff < 0)
				{
					drawingVertexBufferViews.resize(drawingVertexBufferViews.size() + chunkDrawCountDiff);
					drawingIndexBufferViews.resize(drawingIndexBufferViews.size() + chunkDrawCountDiff);
					drawingIndexCounts.resize(drawingIndexCounts.size() + chunkDrawCountDiff);
				}

				// 配列の要素を更新していく
				// 増えた分は push_back で追加していく
				int index = 0;
				for (int chunkX = chunkDrawIndexXMin; chunkX <= chunkDrawIndexXMax; ++chunkX)
					for (int chunkZ = chunkDrawIndexZMin; chunkZ <= chunkDrawIndexZMax; ++chunkZ)
					{
						const VertexBufferView vertexBufferView = terrainVertexBufferViews[chunkX][chunkZ];
						const IndexBufferView indexBufferView = terrainIndexBufferViews[chunkX][chunkZ];
						const int indexCount = static_cast<int>(terrainMeshes[chunkX][chunkZ].indices.size());

						if (index < drawingIndexCounts.size())
						{
							drawingVertexBufferViews[index] = vertexBufferView;
							drawingIndexBufferViews[index] = indexBufferView;
							drawingIndexCounts[index] = indexCount;
						}
						else
						{
							drawingVertexBufferViews.push_back(vertexBufferView);
							drawingIndexBufferViews.push_back(indexBufferView);
							drawingIndexCounts.push_back(indexCount);
						}

						++index;
					}
			}
		}

		const int currentBackRTIndex = D3D12Helper::GetCurrentBackBufferIndex(swapChain);
		const GraphicsBuffer currentBackRT = rtGetter(currentBackRTIndex);
		const DescriptorHeapHandleAtCPU currentBackRTV = rtvGetter(currentBackRTIndex);
		if (!currentBackRT)
			ShowError(L"現在のバックレンダーターゲットの取得に失敗しました");

		// メインレンダリング
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignature, graphicsPipelineState, ppGraphicsBuffer,
			rtvPP, dsv, descriptorHeapBasic, drawingVertexBufferViews, drawingIndexBufferViews,
			GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, RTClearColor, DepthBufferClearValue,
			drawingIndexCounts
		);
		// ポストプロセス
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignaturePP, graphicsPipelineStatePP, textGraphicsBuffer,
			rtvText, dsvPP_Dummy, descriptorHeapBasicPP, { vertexBufferViewPP }, { indexBufferViewPP },
			GraphicsBufferState::Present, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, RTClearColor, DepthBufferClearValue,
			{ static_cast<int>(meshPP.indices.size()) }
		);
		// テキスト描画
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignatureText, graphicsPipelineStateText, currentBackRT,
			currentBackRTV, dsvText_Dummy, descriptorHeapBasicText, { vertexBufferViewText }, { indexBufferViewText },
			GraphicsBufferState::Present, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, RTClearColor, DepthBufferClearValue,
			{ static_cast<int>(meshText.indices.size()) }
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_FRAME;
}
END_INITIALIZE(0);
