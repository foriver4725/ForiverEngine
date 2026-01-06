#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include <scripts/gameFlow/Include.h>
#ifdef _DEBUG
#include <scripts/test/Include.h>
#endif

int Main(hInstance)
{
	using namespace ForiverEngine;

	constexpr Lattice2 WindowSize = Lattice2(1344, 756);
	const HWND hwnd = WindowHelper::OnInit(hInstance, L"ForiverEngine", L"ForiverEngine", WindowSize);

	// テストコード実行
#ifdef _DEBUG
#if 0
	Test::PlayerControl::RunAll();

	ShowError(L"全てのテストに成功しました");
	return 0;
#endif
#endif

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	WindowHelper::SetTargetFps(60);
	WindowHelper::SetCursorEnabled(false);

	constexpr std::uint32_t RandomSeed = 0x12345678;
	Random::SetSeed(RandomSeed);

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
	PlayerController playerController = PlayerController(CameraTransform::CreatePerspective(
		Vector3(64, 32, 64), Quaternion::Identity(), 60.0f * DegToRad, 1.0f * WindowSize.x / WindowSize.y));

	SunCamera sunCamera = SunCamera();
	sunCamera.LookAtPlayer(playerController.GetFootPosition());

	// 地形データ
	Lattice2 existingChunkIndex = Chunk::GetIndex(playerController.GetFootBlockPosition());
	ChunksManager chunksManager = ChunksManager(existingChunkIndex);
	chunksManager.UpdateDrawChunks(existingChunkIndex, false, device); // 初回作成

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
		.Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, playerController.GetTransform()),
		.DirectionalLight_Matrix_VP = sunCamera.CalculateVPMatrix(),
	};
	CBData1 cbData1 =
	{
		.SelectingBlockWorldPosition = Lattice3::Zero(),
		.IsSelectingBlock = 0,
		.SelectColor = Color::CreateFromUint8(255, 255, 0, 100),

		.DirectionalLightDirection = SunCamera::Direction,
		.DirectionalLightColor = Color::White() * 1.2f,
		.AmbientLightColor = Color::White() * 0.5f,

		.CastShadow = 0, // TODO: 影の計算がおかしいので、今は影を無くしておく!
		.ShadowColor = SunCamera::ShadowColor,
	};

	// CBV 用バッファ
	CBData0* cbvBuffer0VirtualPtr = nullptr;
	CBData1* cbvBuffer1VirtualPtr = nullptr;
	const GraphicsBuffer cbvBuffer0 = D3D12BasicFlow::InitCBVBuffer(device, cbData0, &cbvBuffer0VirtualPtr);
	const GraphicsBuffer cbvBuffer1 = D3D12BasicFlow::InitCBVBuffer(device, cbData1, &cbvBuffer1VirtualPtr);

	// SRV 用バッファ
	const Texture textureArray = D3D12BasicFlow::LoadTexture({
			"assets/textures/air_invalid.png",
			"assets/textures/grass_stone.png",
			"assets/textures/dirt_sand.png",
		});
	const auto srvBuffer = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator, textureArray);

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
		.Matrix_MVP = sunCamera.CalculateVPMatrix() * terrainTransform.CalculateModelMatrix(),
	};
	CBData0Shadow* cbvBuffer0ShadowVirtualPtr = nullptr;
	const GraphicsBuffer cbvBufferShadow = D3D12BasicFlow::InitCBVBuffer(device, cbData0Shadow, &cbvBuffer0ShadowVirtualPtr);

	// DescriptorHeap
	const DescriptorHeap descriptorHeapBasicShadow
		= D3D12BasicFlow::InitDescriptorHeapBasic(device, { cbvBufferShadow }, { {shadowGraphicsBuffer, shadowTextureMetadata} });

	const ViewportScissorRect viewportScissorRectShadow = ViewportScissorRect::CreateFullSized(ShadowRTSize);

	//////////
	// メインレンダリングの方に、情報を渡す

	descriptorHeapBasic = D3D12BasicFlow::InitDescriptorHeapBasic(
		device, { cbvBuffer0, cbvBuffer1 }, { {srvBuffer, textureArray}, {shadowGraphicsBuffer, shadowTextureMetadata} });

	//////////

	//////////////////////////////

	//////////////////////////////
	// ポストプロセス

	// b0
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

	const OffscreenRenderer offscreenRendererPP = OffscreenRenderer(
		device, commandList, commandQueue, commandAllocator,
		WindowSize,
		"./shaders/PP.hlsl",
		cbData0PP,
		{}
	);

	//////////////////////////////

	//////////////////////////////
	// テキスト描画 (ポストプロセスの後)

	// テキストUIデータ
	TextUIData textUIData = TextUIData::CreateEmpty(WindowSize / TextUIData::FontTextureTextLength);

	// t1
	const Texture fontTexture = D3D12BasicFlow::LoadTexture({ "assets/font.png" });
	// t2
	Texture textUIDataTexture = textUIData.CreateTexture();

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
		.FontTextureSize = { static_cast<std::uint32_t>(fontTexture.width), static_cast<std::uint32_t>(fontTexture.height) },
		.TextUIDataSize = { static_cast<std::uint32_t>(textUIData.GetDataSize().x), static_cast<std::uint32_t>(textUIData.GetDataSize().y)},
		.InvalidFontTextureIndex = static_cast<std::uint32_t>(Text::InvalidFontTextureIndex),
		.FontTextureTextLength = static_cast<std::uint32_t>(TextUIData::FontTextureTextLength),
	};

	OffscreenRenderer offscreenRendererText = OffscreenRenderer(
		device, commandList, commandQueue, commandAllocator,
		WindowSize,
		"./shaders/Text.hlsl",
		cbData0Text,
		{ fontTexture, textUIDataTexture }
	);

	//////////////////////////////

	while (true)
	{
		if (!WindowHelper::OnBeginFrame(hwnd))
			break;

		// Escape でゲーム終了
		if (InputHelper::GetKeyInfo(Key::Escape).pressedNow)
			break;

		// プレイヤーの挙動
		const PlayerController::Inputs playerInputs =
		{
			.move = InputHelper::GetAsAxis2D(Key::W, Key::S, Key::A, Key::D),
			.look = InputHelper::GetAsAxis2D(Key::Up, Key::Down, Key::Left, Key::Right),
			.dashPressed = InputHelper::GetKeyInfo(Key::LShift).pressed,
			.jumpPressed = InputHelper::GetKeyInfo(Key::Space).pressed,
		};
		playerController.OnEveryFrame(chunksManager.GetChunks(), playerInputs, WindowHelper::GetDeltaSeconds());
		// CB 更新
		cbvBuffer0VirtualPtr->Matrix_MVP = D3D12BasicFlow::CalculateMVPMatrix(terrainTransform, playerController.GetTransform());

		// ブロックを選択する
		{
			const Lattice3 lookingBlockPosition = playerController.PickLookingBlock(chunksManager.GetChunks());

			if (lookingBlockPosition == Lattice3(-1, -1, -1))
			{
				cbvBuffer1VirtualPtr->IsSelectingBlock = 0;
				cbvBuffer1VirtualPtr->SelectingBlockWorldPosition = Lattice3::Zero();
			}
			else
			{
				cbvBuffer1VirtualPtr->IsSelectingBlock = 1;
				cbvBuffer1VirtualPtr->SelectingBlockWorldPosition = lookingBlockPosition;
			}
		}

		// 現在プレイヤーが存在するチャンクのインデックスを取得しておく
		const Lattice2 currentExistingChunkIndex = Chunk::GetIndex(playerController.GetFootBlockPosition());

		// ブロックを壊す
		// チャンクデータを更新し、描画データにも反映させる
		{
			static float mineCooldownTimer = 0.0f;

			if (mineCooldownTimer <= 0.0f && InputHelper::GetKeyInfo(Key::Enter).pressed)
			{
				mineCooldownTimer = PlayerController::MineCooldownSeconds;

				if (cbvBuffer1VirtualPtr->IsSelectingBlock == 1)
				{
					const Lattice2 chunkIndex = Chunk::GetIndex(cbvBuffer1VirtualPtr->SelectingBlockWorldPosition);
					const Lattice3 localBlockPosition
						= Chunk::GetLocalBlockPosition(cbvBuffer1VirtualPtr->SelectingBlockWorldPosition);

					chunksManager.UpdateChunkBlock(chunkIndex, localBlockPosition, Block::Air, device);
					chunksManager.UpdateDrawChunks(currentExistingChunkIndex, true, device);
				}
			}

			mineCooldownTimer -= WindowHelper::GetDeltaSeconds();
			if (mineCooldownTimer < 0.0f)
				mineCooldownTimer = 0.0f;
		}

		// プレイヤーの存在チャンクが変化したなら、描画チャンクを更新する
		if (currentExistingChunkIndex != existingChunkIndex)
		{
			existingChunkIndex = currentExistingChunkIndex;
			chunksManager.UpdateDrawChunks(currentExistingChunkIndex, true, device);
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
				static std::array<double, FrameTimeTextUpdateIntervalFrames> frameTimes = {};
				static int frameTimeTextUpdateCounter = 0;
				static double frameTimeMean = 0.0;
				{
					// データを保存
					frameTimes[frameTimeTextUpdateCounter % FrameTimeTextUpdateIntervalFrames]
						= WindowHelper::GetDeltaMilliseconds();

					// 一定間隔で、平均値を更新
					frameTimeTextUpdateCounter = (frameTimeTextUpdateCounter + 1) % FrameTimeTextUpdateIntervalFrames;
					if (frameTimeTextUpdateCounter == 0)
					{
						double frameTimeSum = 0.0;
						for (const double ft : frameTimes)
							frameTimeSum += ft;

						frameTimeMean = frameTimeSum / FrameTimeTextUpdateIntervalFrames;
					}
				}
				const std::string frameTimeText = std::format("Frame Time : {:.2f} ms", frameTimeMean);
				textUIDataRows.emplace_back(frameTimeText, Color::White());

				// プレイヤーの足元のブロック座標
				const std::string positionText =
					std::format("Position : {}", ToString(playerController.GetFootBlockPosition()));
				textUIDataRows.emplace_back(positionText, Color::White());

				// 選択しているブロックのブロック座標
				const std::string selectingBlockPositionText = (cbvBuffer1VirtualPtr->IsSelectingBlock == 1) ?
					std::format("LookAt : {}", ToString(cbvBuffer1VirtualPtr->SelectingBlockWorldPosition))
					: "LookAt : None";
				textUIDataRows.emplace_back(selectingBlockPositionText, Color::White());

				// 現在いるチャンクのインデックス
				const std::string chunkIndexText = Chunk::IsValidIndex(existingChunkIndex) ?
					std::format("Chunk Index : {}", ToString(existingChunkIndex))
					: "Chunk Index : Invalid";
				textUIDataRows.emplace_back(chunkIndexText, Color::White());

				// チャンク内でのローカルブロック座標
				const std::string chunkLocalBlockPositionText = Chunk::IsValidIndex(existingChunkIndex) ?
					std::format("Chunk Local Position : {}",
						ToString(Chunk::GetLocalBlockPosition(playerController.GetFootBlockPosition())))
					: "Chunk Local Position : Invalid";
				textUIDataRows.emplace_back(chunkLocalBlockPositionText, Color::White());

				// 描画しているチャンクの範囲
				const auto& drawRangeInfo = chunksManager.GetDrawRangeInfo();
				const std::string drawChunksRangeText = std::format(
					"Drawing Chunks : {}-{}",
					ToString(drawRangeInfo.GetRangeMin()),
					ToString(drawRangeInfo.GetRangeMax())
				);
				textUIDataRows.emplace_back(drawChunksRangeText, Color::White());

				// プレイヤーのコリジョンの、ワールドブロック座標の範囲
				const auto [playerCollisionMin, playerCollisionMax] = playerController.GetCollisionRange();
				const std::string playerCollisionRangeText =
					std::format("Player Collision Range : {}-{}",
						ToString(PlayerControl::GetBlockPosition(playerCollisionMin)),
						ToString(PlayerControl::GetBlockPosition(playerCollisionMax))
					);
				textUIDataRows.emplace_back(playerCollisionRangeText, Color::White());

				// 床&天井ブロックのY座標
				const int floorY = playerController.FindFloorHeight(chunksManager.GetChunks());
				const int ceilY = playerController.FindCeilHeight(chunksManager.GetChunks());
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
			offscreenRendererText.ReUploadTexture(device, commandList, commandQueue, commandAllocator,
				textUIDataTexture, ShaderRegister::t2);
		}

		// 太陽カメラの位置を、プレイヤーの頭上らへんにする
		// CBuffer の更新も行う
		{
			sunCamera.LookAtPlayer(playerController.GetFootPosition());

			cbvBuffer0VirtualPtr->DirectionalLight_Matrix_VP = sunCamera.CalculateVPMatrix();
			cbvBuffer0ShadowVirtualPtr->Matrix_MVP = sunCamera.CalculateVPMatrix() * terrainTransform.CalculateModelMatrix();
		}

		const int currentBackRTIndex = D3D12Helper::GetCurrentBackRTIndex(swapChain);
		const GraphicsBuffer currentBackRT = rtGetter(currentBackRTIndex);
		const DescriptorHandleAtCPU currentBackRTV = rtvGetter(currentBackRTIndex);
		if (!currentBackRT)
			ShowError(L"現在のバックレンダーターゲットの取得に失敗しました");

		const auto& packedDrawVBVs = chunksManager.PackDrawVBVs();
		const auto& packedDrawIBVs = chunksManager.PackDrawIBVs();
		const auto& packedDrawMeshIndicesCounts = chunksManager.PackDrawMeshIndicesCounts();

		// 影のデプス書き込み
		if (cbvBuffer1VirtualPtr->CastShadow == 1)
		{
			D3D12BasicFlow::CommandBasicLoop(
				commandList, commandQueue, commandAllocator, device,
				rootSignatureShadow, graphicsPipelineStateShadow, shadowGraphicsBuffer,
				rtvShadow, dsvShadow, descriptorHeapBasicShadow, packedDrawVBVs, packedDrawIBVs,
				GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
				viewportScissorRectShadow, PrimitiveTopology::TriangleList, Color(DepthBufferClearValue, 0, 0, 0), DepthBufferClearValue,
				packedDrawMeshIndicesCounts
			);
		}
		// メインレンダリング
		D3D12BasicFlow::CommandBasicLoop(
			commandList, commandQueue, commandAllocator, device,
			rootSignature, graphicsPipelineState, offscreenRendererPP.GetRT(),
			offscreenRendererPP.GetRTV(), dsv, descriptorHeapBasic, packedDrawVBVs, packedDrawIBVs,
			GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, RTClearColor, DepthBufferClearValue,
			packedDrawMeshIndicesCounts
		);
		// ポストプロセス
		offscreenRendererPP.Draw(
			commandList, commandQueue, commandAllocator, device,
			offscreenRendererText.GetRT(), offscreenRendererText.GetRTV(),
			viewportScissorRect
		);
		// テキスト描画
		offscreenRendererText.Draw(
			commandList, commandQueue, commandAllocator, device,
			currentBackRT, currentBackRTV,
			viewportScissorRect
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");

		WindowHelper::OnEndFrame();
	}

	return 0;
}
