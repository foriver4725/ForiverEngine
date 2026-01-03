#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include <scripts/gameFlow/Include.h>
#if _DEBUG
#include <scripts/test/Include.h>
#endif

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
#if _DEBUG && 0

	Test::PlayerControl::RunAll();

	ShowError(L"全てのテストに成功しました");

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

	constexpr Lattice3 WorldEdgeNoEntryBlockCount = Lattice3(2, 2, 2); // 世界の端から何マス、立ち入り禁止にするか (XYZ方向)
	auto chunkCreationStates = Chunk::CreateChunksArray<std::atomic<ChunkCreationState>>(); // チャンク作成状態 (デフォルト:NotYet)
	auto chunks = Chunk::CreateChunksArray<Chunk>(); // チャンクの配列
	auto chunkMeshes = Chunk::CreateChunksArray<Mesh>(); // 地形の結合メッシュ
	auto chunkVBVs = Chunk::CreateChunksArray<VertexBufferView>(); // 頂点バッファビュー (全部)
	auto chunkIBVs = Chunk::CreateChunksArray<IndexBufferView>(); // インデックスバッファビュー (全部)
	// 地形のデータ・メッシュを作成し、キャッシュしておく関数 (並列処理可能. 最初にこっちを実行する)
	const std::function<void(const Lattice2&)> CreateChunkParallel = [&](const Lattice2& chunkIndex)
		{
			Chunk chunk = Chunk::CreateFromNoise(chunkIndex, { 0.015f, 12.0f }, 16, 18, 24);

			chunkMeshes[chunkIndex.x][chunkIndex.y] = chunk.CreateMesh(chunkIndex);
			chunks[chunkIndex.x][chunkIndex.y] = std::move(chunk);

			chunkCreationStates[chunkIndex.x][chunkIndex.y].store(ChunkCreationState::FinishedParallel, std::memory_order_release);
		};
	// ↑の並列処理を実行開始する関数
	const std::function<void(const Lattice2&)> TryStartCreateChunkParallel = [&](const Lattice2& chunkIndex)
		{
			ChunkCreationState expectedState = ChunkCreationState::NotYet;

			if (!chunkCreationStates[chunkIndex.x][chunkIndex.y]
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
					CreateChunkParallel(chunkIndex);
				}).detach();
		};
	// 地形の頂点・インデックスバッファビューを作成し、キャッシュしておく関数 (GPUが絡むので並列処理不可. 並列処理の方が完了した後、メインスレッドで実行する)
	const std::function<void(const Lattice2&)> CreateChunkNotParallel = [&](const Lattice2& chunkIndex)
		{
			if (chunkCreationStates[chunkIndex.x][chunkIndex.y]
				.load(std::memory_order_acquire)
				!= ChunkCreationState::FinishedParallel)
				return;

			const auto [vertexBufferView, indexBufferView]
				= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, chunkMeshes[chunkIndex.x][chunkIndex.y]);
			chunkVBVs[chunkIndex.x][chunkIndex.y] = vertexBufferView;
			chunkIBVs[chunkIndex.x][chunkIndex.y] = indexBufferView;

			// メインスレッドで1フレーム内で終わらせるので、この状態更新でOK
			chunkCreationStates[chunkIndex.x][chunkIndex.y].store(ChunkCreationState::FinishedAll, std::memory_order_release);
		};

	// 描画するチャンクが変化した時、頂点・インデックスバッファビューを再作成する
	Lattice2 existingChunkIndex = PlayerControl::GetChunkIndex(PlayerControl::GetBlockPosition(cameraTransform.position));
	auto drawChunksIndexRangeInfo = Chunk::GetDrawChunksIndexRangeInfo(existingChunkIndex);
	auto drawChunkVBVs = Chunk::CreateDrawChunksArray<VertexBufferView>();
	auto drawChunkIBVs = Chunk::CreateDrawChunksArray<IndexBufferView>();
	auto drawChunkMeshIndicesCount = Chunk::CreateDrawChunksArray<int>();
	// 更新関数
	// 指定されたチャンクを、描画するものとして配列に登録する
	const std::function<void(const Lattice2&, const Lattice2&)> UpdateDrawChunksRenderDatas =
		[&](const Lattice2& worldChunkIndex, const Lattice2& drawChunksWorldChunkIndexMin)
		{
			const VertexBufferView vbv = chunkVBVs[worldChunkIndex.x][worldChunkIndex.y];
			const IndexBufferView ibv = chunkIBVs[worldChunkIndex.x][worldChunkIndex.y];
			const int indicesCount = static_cast<int>(chunkMeshes[worldChunkIndex.x][worldChunkIndex.y].indices.size());

			const Lattice2 localChunkIndex = worldChunkIndex - drawChunksWorldChunkIndexMin;

			drawChunkVBVs[localChunkIndex.x][localChunkIndex.y] = vbv;
			drawChunkIBVs[localChunkIndex.x][localChunkIndex.y] = ibv;
			drawChunkMeshIndicesCount[localChunkIndex.x][localChunkIndex.y] = indicesCount;
		};

	// チャンク作成状態の初期化
	{
		for (int xi = 0; xi < Chunk::Count; ++xi)
			for (int zi = 0; zi < Chunk::Count; ++zi)
				chunkCreationStates[xi][zi].store(ChunkCreationState::NotYet);
	}

	// 地形の初回作成
	{
		for (const Lattice2& worldChunkIndex : drawChunksIndexRangeInfo)
		{
			// 初回は、メインスレッドで1フレームで全て終わらせる
			CreateChunkParallel(worldChunkIndex);
			CreateChunkNotParallel(worldChunkIndex);

			UpdateDrawChunksRenderDatas(worldChunkIndex, drawChunksIndexRangeInfo.GetRangeMin());
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
		Lattice3 SelectingBlockWorldPosition; // 選択中のブロック位置
		int IsSelectingBlock; // ブロックを選択中かどうか (bool 型として扱う)
		Color SelectColor; // 選択中のブロックの乗算色 (a でブレンド率を指定)

		Vector3 DirectionalLightDirection; // 太陽光の向き (正規化済み)
		float Pad0;
		Color DirectionalLightColor; // 太陽光の色 (a は使わない)
		Color AmbientLightColor; // 環境光の色 (a は使わない)

		int CastShadow; // 影を落とすかどうか (bool 型として扱う)
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
		.SelectingBlockWorldPosition = Lattice3::Zero(),
		.IsSelectingBlock = 0,
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
	const auto [vbvPP, ibvPP]
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
	const auto [vbvText, ibvText]
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
			// 移動前の座標を保存しておく
			const Vector3 positionBeforeMove = cameraTransform.position;

			// 鉛直移動と当たり判定
			{
				// 床のY座標を算出しておく
				const int floorY = PlayerControl::FindFloorHeight(
					chunks, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				// 天井のY座標を算出しておく
				const int ceilY = PlayerControl::FindCeilHeight(
					chunks, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);

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
					(ceilY <= (Chunk::Height - 1)) ?
					((PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight).y + PlayerCollisionSize.y)
						>= (ceilY - 0.5f - CeilingCheckOffset))
					: false;

				// 接地したら、めり込みを補正して、鉛直速度を0にする
				if (isGrounded)
				{
					const float footY = floorY + 0.5f + EyeHeight;
					if (cameraTransform.position.y < footY)
						cameraTransform.position.y = footY;

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
				const Vector3 positionBeforeMoveH = cameraTransform.position;

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
					chunks, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize))
				{
					cameraTransform.position = positionBeforeMoveH;
				}
			}

			// 世界の範囲内に収める
			{
				const Lattice3 footBlockPosition = PlayerControl::GetBlockPosition(
					PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight));

				if (!PlayerControl::IsIntInRange(
					footBlockPosition.x, WorldEdgeNoEntryBlockCount.x, Chunk::Size * Chunk::Count - WorldEdgeNoEntryBlockCount.x) ||
					// TODO: Y方向は上手く判定できないので、一旦無効化
					/*!PlayerControl::IsIntInRange(
						footBlockPosition.y, WorldEdgeNoEntryBlockCount.y, Chunk::Height - WorldEdgeNoEntryBlockCount.y) ||*/
					!PlayerControl::IsIntInRange(
						footBlockPosition.z, WorldEdgeNoEntryBlockCount.z, Chunk::Size * Chunk::Count - WorldEdgeNoEntryBlockCount.z))
				{
					cameraTransform.position = positionBeforeMove;
				}
			}

			// 移動が完了したので、CB 更新
			cbvBuffer0VirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, cameraTransform);
		}

		// ブロックを選択する
		{
			// 値を初期化
			cbvBuffer1VirtualPtr->IsSelectingBlock = 0;
			cbvBuffer1VirtualPtr->SelectingBlockWorldPosition = Lattice3::Zero();

			const Vector3 rayOrigin = cameraTransform.position;
			const Vector3 rayDirection = cameraTransform.GetForward().Normed();

			for (float d = 0.0f; d <= ReachDistance; d += ReachDetectStep)
			{
				const Vector3 rayPosition = rayOrigin + rayDirection * d;
				const Lattice3 rayBlockPosition = PlayerControl::GetBlockPosition(rayPosition);

				const Lattice2 chunkIndex = PlayerControl::GetChunkIndex(rayBlockPosition);
				if (!PlayerControl::IsValidChunkIndex(chunkIndex))
					continue;

				const Chunk& targettingChunk = chunks[chunkIndex.x][chunkIndex.y];
				const Lattice3 rayLocalPosition = PlayerControl::GetChunkLocalPosition(rayBlockPosition);
				const Block blockAtRay = targettingChunk.GetBlock(rayLocalPosition);

				if (blockAtRay != Block::Air)
				{
					cbvBuffer1VirtualPtr->IsSelectingBlock = 1;
					cbvBuffer1VirtualPtr->SelectingBlockWorldPosition = rayBlockPosition;

					break;
				}
			}
		}

		// ブロックを壊す
		// この場合、描画チャンクを必ず更新する
		bool hasBrokenBlock = false;
		{
			if (InputHelper::GetKeyInfo(Key::Enter).pressedNow)
			{
				if (cbvBuffer1VirtualPtr->IsSelectingBlock == 1)
				{
					hasBrokenBlock = true;

					const Lattice2 chunkIndex = PlayerControl::GetChunkIndex(cbvBuffer1VirtualPtr->SelectingBlockWorldPosition);

					// 地形データとメッシュを更新
					chunks[chunkIndex.x][chunkIndex.y].SetBlock(
						PlayerControl::GetChunkLocalPosition(cbvBuffer1VirtualPtr->SelectingBlockWorldPosition), Block::Air);
					chunkMeshes[chunkIndex.x][chunkIndex.y] = chunks[chunkIndex.x][chunkIndex.y].CreateMesh(chunkIndex);

					// 頂点バッファビューとインデックスバッファビューを更新
					const auto [vertexBufferView, indexBufferView]
						= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, chunkMeshes[chunkIndex.x][chunkIndex.y]);
					chunkVBVs[chunkIndex.x][chunkIndex.y] = vertexBufferView;
					chunkIBVs[chunkIndex.x][chunkIndex.y] = indexBufferView;
				}
			}
		}

		// 描画距離内のチャンクのみ、描画する配列に追加
		// プレイヤーの存在するチャンクが変化した、またはブロックを壊した場合に更新する
		// 更新時、そのチャンクがまだ未作成ならば、その作成をまず行う
		{
			// 存在するチャンクが変化したかチェック
			const Lattice2 currentExistingChunkIndex = PlayerControl::GetChunkIndex(PlayerControl::GetBlockPosition(cameraTransform.position));
			const bool hasExistingChunkChanged = (currentExistingChunkIndex != existingChunkIndex);

			// 結局配列を更新する必要があるのか?
			const bool shouldUpdateDrawChunks = hasExistingChunkChanged || hasBrokenBlock;

			if (shouldUpdateDrawChunks)
			{
				// パラメータを更新
				existingChunkIndex = currentExistingChunkIndex;
				const int currentDrawChunkCountTotal = drawChunksIndexRangeInfo.chunkCount;
				drawChunksIndexRangeInfo = Chunk::GetDrawChunksIndexRangeInfo(existingChunkIndex);
				const int drawChunkCountTotalDiff = currentDrawChunkCountTotal - drawChunksIndexRangeInfo.chunkCount;

				// ビューとインデックスを再作成. 配列の要素を更新する
				for (const Lattice2& worldChunkIndex : drawChunksIndexRangeInfo)
				{
					// チャンクが未作成ならば、作成を開始する
					// 作成中にやっぱり描画しないとなっても、スレッドは止まらず並列処理完了まで動き続ける
					// そのため、並列処理でない部分をその後いつ呼んでも問題ない (状態ガードをちゃんと入れているので)
					TryStartCreateChunkParallel(worldChunkIndex);
					CreateChunkNotParallel(worldChunkIndex);

					UpdateDrawChunksRenderDatas(worldChunkIndex, drawChunksIndexRangeInfo.GetRangeMin());
				}
			}
		}

		// テキストの更新
		{
			// データを更新
			{
				static bool hasInitializedData = false;
				// 画面に出すテキスト. 1行ごとに文字を保存
				static std::vector<std::pair<std::string, Color>> textUIDataRows = {};

				if (!hasInitializedData)
				{
					hasInitializedData = true;
					textUIDataRows.reserve(32);
				}
				textUIDataRows.clear();

				// フレームタイム
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
				textUIDataRows.emplace_back(frameTimeText, Color::White());

				// プレイヤーの足元のブロック座標
				const Lattice3 playerFootPositionAsLattice = PlayerControl::GetBlockPosition(
					PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight));
				const std::string positionText =
					std::format("Position : {}", ToString(playerFootPositionAsLattice));
				textUIDataRows.emplace_back(positionText, Color::White());

				// 現在いるチャンクのインデックス
				const std::string chunkIndexText = PlayerControl::IsValidChunkIndex(existingChunkIndex) ?
					std::format("Chunk Index : {}", ToString(existingChunkIndex))
					: "Chunk Index : Invalid";
				textUIDataRows.emplace_back(chunkIndexText, Color::White());

				// チャンク内でのローカルブロック座標
				const std::string chunkLocalBlockPositionText = PlayerControl::IsValidChunkIndex(existingChunkIndex) ?
					std::format("Chunk Local Position : {}",
						ToString(PlayerControl::GetChunkLocalPosition(PlayerControl::GetBlockPosition(cameraTransform.position))))
					: "Chunk Local Position : Invalid";
				textUIDataRows.emplace_back(chunkLocalBlockPositionText, Color::White());

				// 選択しているブロックのブロック座標
				const std::string selectingBlockPositionText = (cbvBuffer1VirtualPtr->IsSelectingBlock == 1) ?
					std::format("LookAt : {}", ToString(cbvBuffer1VirtualPtr->SelectingBlockWorldPosition))
					: "LookAt : None";
				textUIDataRows.emplace_back(selectingBlockPositionText, Color::White());

				// プレイヤーのコリジョンの、ワールドブロック座標の範囲
				const Vector3 playerCollisionMin = PlayerControl::GetCollisionMinPosition(PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				const Vector3 playerCollisionMax = playerCollisionMin + PlayerCollisionSize;
				const Lattice3 playerCollisionMinAsBlock = PlayerControl::GetBlockPosition(playerCollisionMin);
				const Lattice3 playerCollisionMaxAsBlock = PlayerControl::GetBlockPosition(playerCollisionMax);
				const std::string playerCollisionRangeText =
					std::format("Player Collision Range : {}-{}",
						ToString(playerCollisionMinAsBlock),
						ToString(playerCollisionMaxAsBlock)
					);
				textUIDataRows.emplace_back(playerCollisionRangeText, Color::White());

				// 床&天井ブロックのY座標
				const int floorY = PlayerControl::FindFloorHeight(
					chunks, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				const int ceilY = PlayerControl::FindCeilHeight(
					chunks, PlayerControl::GetFootPosition(cameraTransform.position, EyeHeight), PlayerCollisionSize);
				const std::string floorCeilHeightText = std::format(
					"Floor&Ceil Height : ({},{})",
					(floorY >= 0) ? std::to_string(floorY) : "None",
					(ceilY <= Chunk::Height - 1) ? std::to_string(ceilY) : "None"
				);
				textUIDataRows.emplace_back(floorCeilHeightText, Color::White());

				// 本体のデータを更新
				textUIData.ClearAll();
				for (int i = 0; i < static_cast<int>(textUIDataRows.size()); ++i)
				{
					const auto& textUIDataRow = textUIDataRows[i];
					const std::string& text = textUIDataRow.first;
					const Color& color = textUIDataRow.second;

					textUIData.SetTexts(Lattice2(1, i + 1), text, color);
				}
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

		const auto& drawChunkVBVsPacked = Chunk::PackDrawChunksArray(drawChunkVBVs, drawChunksIndexRangeInfo);
		const auto& drawChunkIBVsPacked = Chunk::PackDrawChunksArray(drawChunkIBVs, drawChunksIndexRangeInfo);
		const auto& drawChunkMeshIndicesCountPacked = Chunk::PackDrawChunksArray(drawChunkMeshIndicesCount, drawChunksIndexRangeInfo);

		// 影のデプス書き込み
		if (cbvBuffer1VirtualPtr->CastShadow == 1)
		{
			D3D12BasicFlow::CommandBasicLoop(
				commandList, commandQueue, commandAllocator, device,
				rootSignatureShadow, graphicsPipelineStateShadow, shadowGraphicsBuffer,
				rtvShadow, dsvShadow, descriptorHeapBasicShadow, drawChunkVBVsPacked, drawChunkIBVsPacked,
				GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
				viewportScissorRectShadow, PrimitiveTopology::TriangleList, Color(DepthBufferClearValue, 0, 0, 0), DepthBufferClearValue,
				drawChunkMeshIndicesCountPacked
			);
		}
		// メインレンダリング
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignature, graphicsPipelineState, ppGraphicsBuffer,
			rtvPP, dsv, descriptorHeapBasic, drawChunkVBVsPacked, drawChunkIBVsPacked,
			GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, RTClearColor, DepthBufferClearValue,
			drawChunkMeshIndicesCountPacked
		);
		// ポストプロセス
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignaturePP, graphicsPipelineStatePP, textGraphicsBuffer,
			rtvText, dsvPP_Dummy, descriptorHeapBasicPP, { vbvPP }, { ibvPP },
			GraphicsBufferState::Present, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, Color::Transparent(), DepthBufferClearValue,
			{ static_cast<int>(meshPP.indices.size()) }
		);
		// テキスト描画
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignatureText, graphicsPipelineStateText, currentBackRT,
			currentBackRTV, dsvText_Dummy, descriptorHeapBasicText, { vbvText }, { ibvText },
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
