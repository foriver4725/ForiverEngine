#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include <scripts/gameFlow/Include.h>

int Main(hInstance)
{
	using namespace ForiverEngine;

	constexpr Lattice2 WindowSize = Lattice2(1344, 756);
	const HWND hwnd = WindowHelper::OnInit(hInstance, L"ForiverEngine", L"ForiverEngine", WindowSize);

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	WindowHelper::SetTargetFps(60);
	WindowHelper::SetCursorEnabled(false);

	constexpr std::uint32_t RandomSeed = 0x12345678;
	Random::SetSeed(RandomSeed);

	// テストコード実行
#if 1
	//Test_PlayerControl::RunAll();
#endif

	//////////////////////////////
	// プレイヤー挙動のパラメータ

	constexpr Vector3 PlayerCollisionSize = Vector3(0.5f, 1.8f, 0.5f);
	constexpr float GravityScale = 1.0f; // 重力の倍率
	constexpr float SpeedH = 3.0f; // 水平移動速度 (m/s)
	constexpr float DashSpeedH = 6.0f; // ダッシュ時の水平移動速度 (m/s)
	constexpr float CameraSensitivityH = 180.0f; // 水平感度 (度/s)
	constexpr float CameraSensitivityV = 90.0f; // 垂直感度 (度/s)
	constexpr float MinVelocityV = -100.0f; // 最大落下速度 (m/s)
	constexpr float JumpHeight = 1.3f; // ジャンプ高さ (m)
	constexpr float EyeHeight = 1.6f; // 目の高さ (m)
	constexpr float GroundedCheckOffset = 0.01f; // 接地判定のオフセット (m). 埋まっている判定と区別するため、少しずらす
	constexpr float CeilingCheckOffset = 0.01f; // 天井判定のオフセット (m). 埋まっている判定と区別するため、少しずらす
	float velocityV = 0; // 鉛直速度

	// 向いているブロックを選択
	constexpr float ReachDistance = 5.0f; // 選択可能な最大距離 (m)
	constexpr float ReachDetectStep = 0.1f; // レイキャストの刻み幅 (m)

	//////////////////////////////

	constexpr Color RTClearColor = Color::CreateFromUint8(60, 150, 210); // 空色

	const auto [factory, device, commandAllocator, commandList, commandQueue]
		= D3D12BasicFlow::CreateStandardObjects();

	const RootParameter rootParameter = RootParameter::CreateBasic(2, 2);
	const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVS, shaderPS] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");
	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayouts, FillMode::Solid, CullMode::None, true);

	const SwapChain swapChain = D3D12Helper::CreateSwapChain(factory, commandQueue, hwnd, WindowSize);
	if (!swapChain)
		ShowError(L"SwapChain の作成に失敗しました");
	const auto [rtGetter, rtvGetter] = D3D12BasicFlow::InitRTV(device, swapChain, Format::RGBA_U8_01);
	const DescriptorHandleAtCPU dsv = D3D12BasicFlow::InitDSV(device, WindowSize);

	constexpr Transform terrainTransform = Transform::Identity();
	CameraTransform cameraTransform = CameraTransform::CreatePerspective(
		Vector3(64, 32, 64), Quaternion::Identity(), 60.0f * DegToRad, 1.0f * WindowSize.x / WindowSize.y);

	// 太陽からのカメラ 平行投影
	constexpr Color SunShadowColor = Color(0.7f, 0.7f, 0.7f);
	constexpr float SunDistanceFromPlayer = 100;
	constexpr Vector2 SunClipSizeXY = Vector2(1024, 1024);
	constexpr Vector2 SunClipRangeZ = Vector2(0.1f, 500.0f);
	const Vector3 SunDirection = Vector3(1.0f, -1.0f, 1.0f).Normed();
	const std::function<CameraTransform()> CreateSunCameraTransform = [&]()
		{
			return CameraTransform::CreateOrthographic(
				PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight) - SunDirection * SunDistanceFromPlayer,
				Quaternion::VectorToVector(Vector3::Forward(), SunDirection),
				SunClipSizeXY, SunClipRangeZ
			);
		};
	CameraTransform sunCameraTransform = CreateSunCameraTransform();

	// 地形チャンクの作成進捗状態
	enum class ChunkCreationState : std::uint8_t
	{
		NotYet, // 未作成
		CreatingParallel, // 並列処理中
		FinishedParallel, // 並列処理完了済み
		FinishedAll, // 全部完了済み
	};

	// 地形データ
	constexpr int TerrainSeed = 0x2961E3B1;
	constexpr int ChunkCount = 32; // ワールドのチャンク数 (ChunkCount x ChunkCount 個)
	constexpr int ChunkDrawDistance = 8; // 描画チャンク数 (プレイヤーを中心に 半径 ChunkDrawDistance の矩形内のチャンクのみ描画する)
	constexpr int ChunkDrawMaxCount = (ChunkDrawDistance * 2 + 1) * (ChunkDrawDistance * 2 + 1);
	std::array<std::array<std::atomic<ChunkCreationState>, ChunkCount>, ChunkCount> terrainChunkCreationStates = {}; // チャンク作成状態 (デフォルト:NotYet)
	std::array<std::array<Terrain, ChunkCount>, ChunkCount> terrains = {}; // チャンクの配列
	std::array<std::array<Mesh, ChunkCount>, ChunkCount> terrainMeshes = {}; // 地形の結合メッシュ
	std::array<std::array<VertexBufferView, ChunkCount>, ChunkCount> terrainVertexBufferViews = {}; // 頂点バッファビュー (全部)
	std::array<std::array<IndexBufferView, ChunkCount>, ChunkCount> terrainIndexBufferViews = {}; // インデックスバッファビュー (全部)
	// 地形のデータ・メッシュを作成し、キャッシュしておく関数 (並列処理可能. 最初にこっちを実行する)
	const std::function<void(int, int)> CreateTerrainChunkCanParallel = [&](int chunkX, int chunkZ)
		{
			const Terrain terrain = Terrain::CreateFromNoise({ chunkX, chunkZ }, { 0.015f, 12.0f }, TerrainSeed, 16, 18, 24);
			terrains[chunkX][chunkZ] = terrain;

			const Lattice2 localOffset = Lattice2(chunkX * Terrain::ChunkSize, chunkZ * Terrain::ChunkSize);
			terrainMeshes[chunkX][chunkZ] = terrain.CreateMesh(localOffset);

			terrainChunkCreationStates[chunkX][chunkZ].store(ChunkCreationState::FinishedParallel, std::memory_order_release);
		};
	// ↑の並列処理を実行開始する関数
	const std::function<void(int, int)> TryStartCreateTerrainChunkCanParallel = [&](int chunkX, int chunkZ)
		{
			ChunkCreationState expectedState = ChunkCreationState::NotYet;

			if (!terrainChunkCreationStates[chunkX][chunkZ]
				.compare_exchange_strong(
					expectedState,
					ChunkCreationState::CreatingParallel,
					std::memory_order_acq_rel))
			{
				// 既に該当処理が開始済みなので、何もしない
				return;
			}

			std::thread([=]()
				{
					CreateTerrainChunkCanParallel(chunkX, chunkZ);
				}).detach();
		};
	// 地形の頂点・インデックスバッファビューを作成し、キャッシュしておく関数 (GPUが絡むので並列処理不可. 並列処理の方が完了した後、メインスレッドで実行する)
	const std::function<void(int, int)> CreateTerrainChunkCannotParallel = [&](int chunkX, int chunkZ)
		{
			if (terrainChunkCreationStates[chunkX][chunkZ]
				.load(std::memory_order_acquire)
				!= ChunkCreationState::FinishedParallel)
				return;

			const auto [vertexBufferView, indexBufferView]
				= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, terrainMeshes[chunkX][chunkZ]);
			terrainVertexBufferViews[chunkX][chunkZ] = vertexBufferView;
			terrainIndexBufferViews[chunkX][chunkZ] = indexBufferView;

			// メインスレッドで1フレーム内で終わらせるので、この状態更新でOK
			terrainChunkCreationStates[chunkX][chunkZ]
				.store(ChunkCreationState::FinishedAll, std::memory_order_release);
		};
	// 描画するチャンクが変化した時、ビューを再作成する
	Lattice2 chunkIndex = PlayerControl::GetChunkIndex(PlayerControl::GetBlockPosition(cameraTransform.position));
	Lattice2 chunkDrawIndexRangeX =
		Lattice2(std::max(0, chunkIndex.x - ChunkDrawDistance), std::min(ChunkCount - 1, chunkIndex.x + ChunkDrawDistance));
	Lattice2 chunkDrawIndexRangeZ =
		Lattice2(std::max(0, chunkIndex.y - ChunkDrawDistance), std::min(ChunkCount - 1, chunkIndex.y + ChunkDrawDistance));
	// 一応計算しておく (<= chunkDrawMaxCount)
	int chunkDrawCount = (chunkDrawIndexRangeX.y - chunkDrawIndexRangeX.x + 1) * (chunkDrawIndexRangeZ.y - chunkDrawIndexRangeZ.x + 1);
	// 描画するビューとインデックス
	std::vector<VertexBufferView> drawingVertexBufferViews; drawingVertexBufferViews.reserve(ChunkDrawMaxCount);
	std::vector<IndexBufferView> drawingIndexBufferViews; drawingIndexBufferViews.reserve(ChunkDrawMaxCount);
	std::vector<int> drawingIndexCounts; drawingIndexCounts.reserve(ChunkDrawMaxCount);

	// チャンク作成状態の初期化
	{
		for (auto& zArray : terrainChunkCreationStates)
			for (auto& value : zArray)
				value.store(ChunkCreationState::NotYet);
	}

	// 地形の初回作成
	{
		for (int chunkX = chunkDrawIndexRangeX.x; chunkX <= chunkDrawIndexRangeX.y; ++chunkX)
			for (int chunkZ = chunkDrawIndexRangeZ.x; chunkZ <= chunkDrawIndexRangeZ.y; ++chunkZ)
			{
				// 初回は、メインスレッドで1フレームで全て終わらせる
				CreateTerrainChunkCanParallel(chunkX, chunkZ);
				CreateTerrainChunkCannotParallel(chunkX, chunkZ);

				const VertexBufferView vertexBufferView = terrainVertexBufferViews[chunkX][chunkZ];
				const IndexBufferView indexBufferView = terrainIndexBufferViews[chunkX][chunkZ];
				const int indexCount = static_cast<int>(terrainMeshes[chunkX][chunkZ].indices.size());

				drawingVertexBufferViews.push_back(vertexBufferView);
				drawingIndexBufferViews.push_back(indexBufferView);
				drawingIndexCounts.push_back(indexCount);
			}
	}

	// b0
	struct alignas(256) CBData0
	{
		Matrix4x4 Matrix_M; // M
		Matrix4x4 Matrix_M_IT; // M の逆→転置行列
		Matrix4x4 Matrix_MVP; // MVP
		Matrix4x4 DirectionalLight_Matrix_VP; // 太陽カメラの VP
	};
	// b1
	struct alignas(256) CBData1
	{
		Vector3 SelectingBlockPosition; // 選択中のブロック位置 (ワールド座標)
		float IsSelectingAnyBlock; // ブロックを選択中かどうか (bool 型として扱う)
		Color SelectColor; // 選択中のブロックの乗算色 (a でブレンド率を指定)

		Vector3 DirectionalLightDirection; // 太陽光の向き (正規化済み)
		float Pad0;
		Color DirectionalLightColor; // 太陽光の色 (a は使わない)
		Color AmbientLightColor; // 環境光の色 (a は使わない)

		float CastShadow; // 影を落とすかどうか (bool 型として扱う)
		float Pad1[3];
		Color ShadowColor; // 影の色 (色係数. a は使わない)
	};

	CBData0 cbData0 =
	{
		.Matrix_M = terrainTransform.CalculateModelMatrix(),
		.Matrix_M_IT = terrainTransform.CalculateModelMatrixInversed().Transposed(),
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform),
		.DirectionalLight_Matrix_VP = sunCameraTransform.CalculateVPMatrix(),
	};
	CBData1 cbData1 =
	{
		.SelectingBlockPosition = Vector3::Zero(),
		.IsSelectingAnyBlock = 0,
		.SelectColor = Color::CreateFromUint8(255, 255, 0, 100),

		.DirectionalLightDirection = SunDirection,
		.DirectionalLightColor = Color::White() * 1.2f,
		.AmbientLightColor = Color::White() * 0.5f,

		.CastShadow = 0, // TODO: 影の計算がおかしいので、今は影を無くしておく!
		.ShadowColor = SunShadowColor,
	};

	// CBV 用バッファ
	CBData0* cbvBuffer0VirtualPtr = nullptr;
	CBData1* cbvBuffer1VirtualPtr = nullptr;
	const GraphicsBuffer cbvBuffer0 = D3D12BasicFlow::InitCBVBuffer<CBData0>(device, cbData0, &cbvBuffer0VirtualPtr);
	const GraphicsBuffer cbvBuffer1 = D3D12BasicFlow::InitCBVBuffer<CBData1>(device, cbData1, &cbvBuffer1VirtualPtr);

	// SRV 用バッファ
	const auto srvBufferAndData = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator,
		{
			"assets/textures/air_invalid.png",
			"assets/textures/grass_stone.png",
			"assets/textures/dirt_sand.png",
		});
	const Texture textureArrayMetadata = std::get<1>(srvBufferAndData);

	// DescriptorHeap に登録
	DescriptorHeap descriptorHeapBasic = DescriptorHeap(); // 後で作成する!!

	const ViewportScissorRect viewportScissorRect = ViewportScissorRect::CreateFullSized(WindowSize);

	//////////////////////////////
	// 影 デプスマップに出力

	constexpr Lattice2 ShadowRTSize = Lattice2(1024, 1024);

	const Texture shadowTextureMetadata = Texture::CreateManually({}, ShadowRTSize, Format::R_F32);
	const GraphicsBuffer shadowGraphicsBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, shadowTextureMetadata,
		GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color(DepthBufferClearValue, 0, 0, 0));

	const RootParameter rootParameterShadow = RootParameter::CreateBasic(1, 1);
	const SamplerConfig samplerConfigShadow = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVSShadow, shaderPSShadow] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/ShadowDepthWrite.hlsl");
	const auto [rootSignatureShadow, graphicsPipelineStateShadow]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameterShadow, samplerConfigShadow, shaderVSShadow, shaderPSShadow, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, true);

	// RTV, DSV
	const DescriptorHandleAtCPU rtvShadow = D3D12BasicFlow::InitRTV(device, shadowGraphicsBuffer, Format::R_F32);
	const DescriptorHandleAtCPU dsvShadow = D3D12BasicFlow::InitDSV(device, ShadowRTSize);

	// CB 0
	struct alignas(256) CBData0Shadow
	{
		Matrix4x4 Matrix_MVP;
	};
	CBData0Shadow cbData0Shadow =
	{
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, sunCameraTransform),
	};
	CBData0Shadow* cbvBuffer0ShadowVirtualPtr = nullptr;
	const GraphicsBuffer cbvBufferShadow = D3D12BasicFlow::InitCBVBuffer<CBData0Shadow>(device, cbData0Shadow, &cbvBuffer0ShadowVirtualPtr);

	// DescriptorHeap
	const DescriptorHeap descriptorHeapBasicShadow
		= D3D12BasicFlow::InitDescriptorHeapBasic(device, { cbvBufferShadow }, { {shadowGraphicsBuffer, shadowTextureMetadata} });

	const ViewportScissorRect viewportScissorRectShadow = ViewportScissorRect::CreateFullSized(ShadowRTSize);

	//////////
	// メインレンダリングの方に、情報を渡す

	descriptorHeapBasic = D3D12BasicFlow::InitDescriptorHeapBasic(
		device, { cbvBuffer0, cbvBuffer1 }, { srvBufferAndData, { shadowGraphicsBuffer, shadowTextureMetadata } });

	//////////

	//////////////////////////////

	//////////////////////////////
	// ポストプロセス

	const Texture ppTextureMetadata = Texture::CreateManually({}, WindowSize, Format::RGBA_U8_01);
	// RT, SR で切り替えて使う 2D テクスチャ
	const GraphicsBuffer ppGraphicsBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, ppTextureMetadata,
		GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color::Transparent());

	const RootParameter rootParameterPP = RootParameter::CreateBasic(1, 1);
	const SamplerConfig samplerConfigPP = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVSPP, shaderPSPP] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/PP.hlsl");
	const auto [rootSignaturePP, graphicsPipelineStatePP]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameterPP, samplerConfigPP, shaderVSPP, shaderPSPP, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, false);

	// RTVのみ作成
	const DescriptorHandleAtCPU rtvPP = D3D12BasicFlow::InitRTV(device, ppGraphicsBuffer, Format::RGBA_U8_01);
	const DescriptorHandleAtCPU dsvPP_Dummy = DescriptorHandleAtCPU{ .ptr = NULL };

	// 板ポリのメッシュ
	const MeshQuad meshPP = MeshQuad::CreateFullSized();

	// 頂点バッファビューとインデックスバッファビュー
	const auto [vertexBufferViewPP, indexBufferViewPP]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, meshPP);

	// CB 0
	struct alignas(256) CBData0PP
	{
		std::uint32_t WindowSize[2];
		float LimitLuminance; // ピクセルがモデルの端にあると判断する輝度差の閾値 ([0.0, 1.0]. 小さいほどAAが多くかかる)
		float AAPower; // アンチエイリアスの強さ (大きいほどAAが強くかかる)
	};
	const CBData0PP cbData0PP =
	{
		.WindowSize = { static_cast<std::uint32_t>(WindowSize.x), static_cast<std::uint32_t>(WindowSize.y) },
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

	// RT と同じ. sRGB 不可.
	const Texture textTextureMetadata = Texture::CreateManually({}, WindowSize, Format::RGBA_U8_01);
	// RT, SR で切り替えて使う 2D テクスチャ
	const GraphicsBuffer textGraphicsBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, textTextureMetadata,
		GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color::Transparent());

	const RootParameter rootParameterText = RootParameter::CreateBasic(1, 3);
	const SamplerConfig samplerConfigText = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVSText, shaderPSText] = D3D12BasicFlow::CompileShader_VS_PS("./shaders/Text.hlsl");
	const auto [rootSignatureText, graphicsPipelineStateText]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameterText, samplerConfigText, shaderVSText, shaderPSText, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, false);

	// RTVのみ作成
	const DescriptorHandleAtCPU rtvText = D3D12BasicFlow::InitRTV(device, textGraphicsBuffer, Format::RGBA_U8_01);
	const DescriptorHandleAtCPU dsvText_Dummy = DescriptorHandleAtCPU{ .ptr = NULL };

	// 板ポリのメッシュ
	const MeshQuad meshText = MeshQuad::CreateFullSized();

	// 頂点バッファビューとインデックスバッファビュー
	const auto [vertexBufferViewText, indexBufferViewText]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, meshText);

	// テキストUIデータ
	TextUIData textUIData = TextUIData::CreateEmpty(WindowSize / TextUIData::FontTextureTextLength);

	// t1
	const auto fontTextureBufferAndData = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator, { "assets/font.png" });
	// t2
	Texture textUIDataTexture = textUIData.CreateTexture();
	GraphicsBuffer textUIDataGraphicsBuffer = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator, textUIDataTexture);

	// b0
	struct alignas(256) CBData0Text
	{
		std::uint32_t FontTextureSize[2];
		std::uint32_t TextUIDataSize[2];
		std::uint32_t InvalidFontTextureIndex;
		std::uint32_t FontTextureTextLength;
	};
	const CBData0Text cbData0Text = CBData0Text
	{
		.FontTextureSize = { static_cast<std::uint32_t>(std::get<1>(fontTextureBufferAndData).width), static_cast<std::uint32_t>(std::get<1>(fontTextureBufferAndData).height) },
		.TextUIDataSize = { static_cast<std::uint32_t>(textUIData.GetDataSize().x), static_cast<std::uint32_t>(textUIData.GetDataSize().y)},
		.InvalidFontTextureIndex = static_cast<std::uint32_t>(Text::InvalidFontTextureIndex),
		.FontTextureTextLength = static_cast<std::uint32_t>(TextUIData::FontTextureTextLength),
	};
	const GraphicsBuffer cbvBufferText = D3D12BasicFlow::InitCBVBuffer<CBData0Text>(device, cbData0Text);

	// DescriptorHeap
	const DescriptorHeap descriptorHeapBasicText
		= D3D12BasicFlow::InitDescriptorHeapBasic(
			device,
			{ cbvBufferText },
			{
				{ textGraphicsBuffer, textTextureMetadata },
				fontTextureBufferAndData,
				{ textUIDataGraphicsBuffer, textUIDataTexture },
			}
			);

	//////////////////////////////

	while (true)
	{
		if (!WindowHelper::OnBeginFrame(hwnd))
			break;

		// Escape でゲーム終了
		if (InputHelper::GetKeyInfo(Key::Escape).pressedNow)
			break;

		// 回転
		cameraTransform.rotation = PlayerControl::Rotate(
			cameraTransform,
			InputHelper::GetAsAxis2D(Key::Up, Key::Down, Key::Left, Key::Right),
			Vector2(CameraSensitivityH, CameraSensitivityV) * DegToRad,
			WindowHelper::GetDeltaSeconds<float>()
		);

		// 移動
		{
			// 鉛直移動と当たり判定
			{
				// 床のY座標を算出しておく
				const int floorY = PlayerControl::FindFloorHeight(
					terrains, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				// 天井のY座標を算出しておく
				const int ceilY = PlayerControl::FindCeilHeight(
					terrains, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);

				// 落下分の加速度を加算し、鉛直移動する
				velocityV -= (G * GravityScale) * WindowHelper::GetDeltaSeconds<float>();
				velocityV = std::max(velocityV, MinVelocityV);
				if (std::abs(velocityV) > 0.01f)
					cameraTransform.position += Vector3::Up() * (velocityV * WindowHelper::GetDeltaSeconds<float>());

				// 設置判定
				const bool isGrounded =
					(floorY >= 0) ?
					(PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight).y
						<= (floorY + 0.5f + GroundedCheckOffset))
					: false;
				// 天井判定
				const bool isCeiling =
					(ceilY <= (Terrain::ChunkHeight - 1)) ?
					((PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight).y + PlayerCollisionSize.y)
						>= (ceilY - 0.5f - CeilingCheckOffset))
					: false;

				// 接地したら、めり込みを補正して、鉛直速度を0にする
				if (isGrounded)
				{
					const float standY = floorY + 0.5f + EyeHeight;
					if (cameraTransform.position.y < standY)
						cameraTransform.position.y = standY;

					if (velocityV < 0)
						velocityV = 0;
				}
				// 天井にぶつかったら、めり込みを補正して、鉛直速度を0にする
				else if (isCeiling)
				{
					const float headY = ceilY - 0.5f - PlayerCollisionSize.y + EyeHeight;
					if (cameraTransform.position.y > headY)
						cameraTransform.position.y = headY;

					if (velocityV > 0)
						velocityV = 0;
				}

				// 接地しているなら、ジャンプ入力を受け付ける
				if (isGrounded)
				{
					if (InputHelper::GetKeyInfo(Key::Space).pressedNow)
						velocityV += std::sqrt(2.0f * G * JumpHeight);
				}
			}

			// 水平移動と当たり判定
			{
				// 移動前の座標を保存しておく
				const Vector3 positionBeforeMove = cameraTransform.position;

				const Vector2 moveInput = InputHelper::GetAsAxis2D(Key::W, Key::S, Key::A, Key::D);
				const bool canDash = moveInput.y > 0.5f; // 前進しているときのみダッシュ可能

				cameraTransform.position = PlayerControl::MoveH(
					cameraTransform,
					moveInput,
					(canDash && InputHelper::GetKeyInfo(Key::LShift).pressed) ? DashSpeedH : SpeedH,
					WindowHelper::GetDeltaSeconds<float>()
				);

				// 当たり判定
				// めり込んでいるなら、元の位置に戻す
				if (PlayerControl::IsOverlappingWithTerrain(
					terrains,
					PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight),
					PlayerCollisionSize))
				{
					cameraTransform.position = positionBeforeMove;
				}
			}

			// 移動が完了したので、CB 更新
			cbvBuffer0VirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform);
		}

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

				const Lattice2 chunkIndex = PlayerControl::GetChunkIndex(PlayerControl::GetBlockPosition(rayPosition));
				if (!PlayerControl::IsValidChunkIndex(chunkIndex, ChunkCount))
					continue;
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

			cbvBuffer1VirtualPtr->IsSelectingAnyBlock = (hitBlock != Block::Air) ? 1.0f : 0.0f;
			if (hitBlock != Block::Air)
			{
				cbvBuffer1VirtualPtr->SelectingBlockPosition = Vector3(
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
				if (cbvBuffer1VirtualPtr->IsSelectingAnyBlock > 0.5f)
				{
					hasBrokenBlock = true;

					const Lattice3 blockPositionAsLattice = Lattice3(cbvBuffer1VirtualPtr->SelectingBlockPosition);
					const Lattice2 chunkIndex = PlayerControl::GetChunkIndex(PlayerControl::GetBlockPosition(cbvBuffer1VirtualPtr->SelectingBlockPosition));

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
		// 更新時、そのチャンクがまだ未作成ならば、その作成をまず行う
		{
			// 存在するチャンクが変化したかチェック
			const Lattice2 currentChunkIndex = PlayerControl::GetChunkIndex(PlayerControl::GetBlockPosition(cameraTransform.position));
			const bool hasExistingChunkChanged = (currentChunkIndex != chunkIndex);

			// 結局配列を更新する必要があるのか?
			const bool shouldUpdateDrawChunks = hasExistingChunkChanged || hasBrokenBlock;

			if (shouldUpdateDrawChunks)
			{
				// パラメータを更新
				chunkIndex = currentChunkIndex;
				chunkDrawIndexRangeX =
					Lattice2(std::max(0, chunkIndex.x - ChunkDrawDistance), std::min(ChunkCount - 1, chunkIndex.x + ChunkDrawDistance));
				chunkDrawIndexRangeZ =
					Lattice2(std::max(0, chunkIndex.y - ChunkDrawDistance), std::min(ChunkCount - 1, chunkIndex.y + ChunkDrawDistance));
				// この後配列を更新するので、サイズが変わったかどうか保存しておく
				const int currentChunkDrawCount = (chunkDrawIndexRangeX.y - chunkDrawIndexRangeX.x + 1) * (chunkDrawIndexRangeZ.y - chunkDrawIndexRangeZ.x + 1);
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
				for (int chunkX = chunkDrawIndexRangeX.x; chunkX <= chunkDrawIndexRangeX.y; ++chunkX)
					for (int chunkZ = chunkDrawIndexRangeZ.x; chunkZ <= chunkDrawIndexRangeZ.y; ++chunkZ)
					{
						// チャンクが未作成ならば、作成を開始する
						// 作成中にやっぱり描画しないとなっても、スレッドは止まらず並列処理完了まで動き続ける
						// そのため、並列処理でない部分をその後いつ呼んでも問題ない (状態ガードをちゃんと入れているので)
						TryStartCreateTerrainChunkCanParallel(chunkX, chunkZ);
						CreateTerrainChunkCannotParallel(chunkX, chunkZ);

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

		// テキストの更新
		{
			// データを更新
			{
				// - フレームタイム
				// - プレイヤーの足元のブロック座標
				// - 現在いるチャンクのインデックス
				// - チャンク内でのローカルブロック座標
				// - 選択しているブロックのブロック座標
				// - プレイヤーのコリジョンの、ワールドブロック座標の範囲
				// - プレイヤーのXZ座標について、Floor にあるブロックのY座標
				// - プレイヤーのXZ座標について、Ceil  にあるブロックのY座標

				constexpr int FrameTimeTextUpdateIntervalFrames = 16; // テキストの更新間隔 (フレーム数)
				static int frameTimeTextUpdateCounter = 0;
				static double frameTimeTextValue = 0.0;
				{
					// フレーム数をカウントし、一定周期でフレームタイムを更新する
					if (++frameTimeTextUpdateCounter >= FrameTimeTextUpdateIntervalFrames)
						frameTimeTextUpdateCounter = 0;
					if (frameTimeTextUpdateCounter % FrameTimeTextUpdateIntervalFrames == 0)
						frameTimeTextValue = WindowHelper::GetDeltaMilliseconds<double>();
				}
				const std::string frameTimeText = std::format("Frame Time : {:.2f} ms", frameTimeTextValue);

				const Lattice3 playerFootPositionAsLattice = PlayerControl::GetBlockPosition(
					PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight));
				const std::string positionText =
					std::format("Position : {}", ToString(playerFootPositionAsLattice));

				const Lattice2 currentChunkIndex = PlayerControl::GetChunkIndex(PlayerControl::GetBlockPosition(cameraTransform.position));
				const bool isCurrentChunkIndexValid = PlayerControl::IsValidChunkIndex(currentChunkIndex, ChunkCount);
				const std::string chunkIndexText = isCurrentChunkIndexValid ?
					std::format("Chunk Index : {}", ToString(currentChunkIndex))
					: "Chunk Index : Invalid";

				const Lattice3 currentChunkLocalBlockPosition = isCurrentChunkIndexValid ?
					PlayerControl::GetChunkLocalPosition(PlayerControl::GetBlockPosition(cameraTransform.position))
					: Lattice3::Zero();
				const std::string chunkLocalBlockPositionText = isCurrentChunkIndexValid ?
					std::format("Chunk Local Position : {}", ToString(currentChunkLocalBlockPosition))
					: "Chunk Local Position : Invalid";

				const Lattice3 selectingBlockPositionAsLattice = PlayerControl::GetBlockPosition(
					cbvBuffer1VirtualPtr->SelectingBlockPosition);
				const std::string selectingBlockPositionText = cbvBuffer1VirtualPtr->IsSelectingAnyBlock ?
					std::format("LookAt : {}", ToString(selectingBlockPositionAsLattice))
					: "LookAt : None";

				const Vector3 playerCollisionMin = PlayerControl::GetCollisionMinPosition(PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				const Vector3 playerCollisionMax = playerCollisionMin + PlayerCollisionSize;
				const Lattice3 playerCollisionMinAsBlock = PlayerControl::GetBlockPosition(playerCollisionMin);
				const Lattice3 playerCollisionMaxAsBlock = PlayerControl::GetBlockPosition(playerCollisionMax);
				const std::string playerCollisionRangeText =
					std::format("Player Collision Range : {} - {}",
						ToString(playerCollisionMinAsBlock),
						ToString(playerCollisionMaxAsBlock)
					);

				const int floorY = PlayerControl::FindFloorHeight(terrains, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				const std::string floorYText = (floorY >= 0) ?
					std::format("Floor Y : {}", floorY)
					: "Floor Y : None";

				const int ceilY = PlayerControl::FindCeilHeight(terrains, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				const std::string ceilYText = (ceilY <= Terrain::ChunkHeight - 1) ?
					std::format("Ceil Y : {}", ceilY)
					: "Ceil Y : None";

				textUIData.ClearAll();
				textUIData.SetTexts(Lattice2(1, 1), frameTimeText, Color::White());
				textUIData.SetTexts(Lattice2(1, 2), positionText, Color::White());
				textUIData.SetTexts(Lattice2(1, 3), chunkIndexText, Color::White());
				textUIData.SetTexts(Lattice2(1, 4), chunkLocalBlockPositionText, Color::White());
				textUIData.SetTexts(Lattice2(1, 5), selectingBlockPositionText, Color::White());
				textUIData.SetTexts(Lattice2(1, 6), playerCollisionRangeText, Color::White());
				textUIData.SetTexts(Lattice2(1, 7), floorYText, Color::White());
				textUIData.SetTexts(Lattice2(1, 8), ceilYText, Color::White());
			}

			// バッファを再作成してアップロードし直す
			textUIDataTexture = textUIData.CreateTexture();
			textUIDataGraphicsBuffer = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator, textUIDataTexture);
			D3D12Helper::CreateSRVAndRegistToDescriptorHeap(
				device, descriptorHeapBasicText,
				textUIDataGraphicsBuffer,
				static_cast<int>(ShaderRegister::t2) + 1, // CBVが1つあるので...
				textUIDataTexture
			);
		}

		// 太陽カメラの位置を、プレイヤーの頭上らへんにする
		// CBuffer の更新も行う
		{
			sunCameraTransform = CreateSunCameraTransform();

			cbvBuffer0VirtualPtr->DirectionalLight_Matrix_VP = sunCameraTransform.CalculateVPMatrix();
			cbvBuffer0ShadowVirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, sunCameraTransform);
		}

		const int currentBackRTIndex = D3D12Helper::GetCurrentBackRTIndex(swapChain);
		const GraphicsBuffer currentBackRT = rtGetter(currentBackRTIndex);
		const DescriptorHandleAtCPU currentBackRTV = rtvGetter(currentBackRTIndex);
		if (!currentBackRT)
			ShowError(L"現在のバックレンダーターゲットの取得に失敗しました");

		// 影のデプス書き込み
		if (cbvBuffer1VirtualPtr->CastShadow > 0.5f)
		{
			D3D12BasicFlow::CommandBasicLoop(
				commandList, commandQueue, commandAllocator, device,
				rootSignatureShadow, graphicsPipelineStateShadow, shadowGraphicsBuffer,
				rtvShadow, dsvShadow, descriptorHeapBasicShadow, drawingVertexBufferViews, drawingIndexBufferViews,
				GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
				viewportScissorRectShadow, PrimitiveTopology::TriangleList, Color(DepthBufferClearValue, 0, 0, 0), DepthBufferClearValue,
				drawingIndexCounts
			);
		}
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
			viewportScissorRect, PrimitiveTopology::TriangleList, Color::Transparent(), DepthBufferClearValue,
			{ static_cast<int>(meshPP.indices.size()) }
		);
		// テキスト描画
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignatureText, graphicsPipelineStateText, currentBackRT,
			currentBackRTV, dsvText_Dummy, descriptorHeapBasicText, { vertexBufferViewText }, { indexBufferViewText },
			GraphicsBufferState::Present, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, Color::Transparent(), DepthBufferClearValue,
			{ static_cast<int>(meshText.indices.size()) }
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");

		WindowHelper::OnEndFrame();
	}

	return 0;
}
