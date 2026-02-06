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
	const HWND hwnd = WindowHelper::OnInit(hInstance, WindowSize);

	// テストコード実行
#ifdef _DEBUG
#if 0
	Test::PlayerControl::RunAll();

	ShowError(L"全てのテストに成功しました");
	return 0;
#endif
#endif

	// シェーダーのコンパイル (開発中のみ)
#ifdef _DEBUG
#if 0
	if (D3D12Helper::IDE_CompileHlslToCso(
		R"(C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe)", // 開発者ごとに違う可能性がある. 適宜変えてほしい
		"shaders", ".compiledShaderObjects", "shaders",
		{
			"Basic.hlsl",
			"ShadowDepthWrite.hlsl",
			"PP.hlsl",
			"Text.hlsl",
			"Image.hlsl"
		}
	))
	{
		ShowError(L"シェーダーのコンパイルに成功しました");
		return 0;

	}
	else
	{
		ShowError(L"シェーダーのコンパイルに失敗しました");
		return -1;
	}
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

	const auto [factory, device, commandAllocator, commandList, commandQueue]
		= D3D12Utils::CreateStandardObjects();



	// プレイヤーが存在するチャンクのインデックス
	constexpr Lattice2 playerInitChunkIndex = Lattice2(Chunk::Count / 2, Chunk::Count / 2); // 初期スポーン地点は、ワールドのど真ん中
	TrackedValue<Lattice2> playerExistingChunkIndex = TrackedValue(playerInitChunkIndex);

	// 地形データ
	ChunksManager chunksManager = ChunksManager(playerExistingChunkIndex.GetValue());
	chunksManager.UpdateDrawChunks(playerExistingChunkIndex.GetValue(), false, device); // 初回作成

	// プレイヤーコントローラー
	PlayerController playerController = PlayerController(WindowSize, playerExistingChunkIndex.GetValue(), chunksManager.GetChunks());

	// 太陽カメラ
	SunCamera sunCamera = SunCamera();
	sunCamera.LookAtPlayer(playerController.GetFootPosition());

	const SwapChain swapChain = D3D12Helper::CreateSwapChain(factory, commandQueue, hwnd, WindowSize);
	if (!swapChain)
		ShowError(L"SwapChain の作成に失敗しました");
	const auto [rtGetter, rtvGetter] = D3D12Utils::InitRTV(device, swapChain, Format::RGBA_U8_01);

	const ViewportScissorRect viewportScissorRect = ViewportScissorRect::CreateFullSized(WindowSize);

#pragma region Shadow

	constexpr Lattice2 ShadowRTSize = Lattice2(1024, 1024);

	const Texture shadowTextureMetadata = Texture({}, Lattice3(ShadowRTSize, 1), Format::R_F32);
	const GraphicsBuffer shadowGraphicsBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, shadowTextureMetadata,
		GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color(DepthBufferClearValue, 0, 0, 0));

	const RootParameter rootParameterShadow = RootParameter::CreateBasic(1, 1);
	const SamplerConfig samplerConfigShadow = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVSShadow, shaderPSShadow] = D3D12Utils::LoadCso(D3D12Utils::GetShaderFilePath("ShadowDepthWrite"));
	const auto [rootSignatureShadow, graphicsPipelineStateShadow]
		= D3D12Utils::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameterShadow, samplerConfigShadow, shaderVSShadow, shaderPSShadow, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, true);

	// RTV, DSV
	const DescriptorHandleAtCPU rtvShadow = D3D12Utils::InitRTV(device, shadowGraphicsBuffer, Format::R_F32);
	const DescriptorHandleAtCPU dsvShadow = D3D12Utils::InitDSV(device, ShadowRTSize);

	// b0
	struct alignas(256) CBData0Shadow
	{
		Matrix4x4 Matrix_MVP;
	};
	CBData0Shadow cbData0Shadow =
	{
		.Matrix_MVP = sunCamera.CalculateVPMatrix() * TerrainRenderer::Transform.CalculateModelMatrix(),
	};
	CBData0Shadow* cb0ShadowVirtualPtr = nullptr;
	const GraphicsBuffer cb0Shadow = D3D12Utils::InitCB(device, cbData0Shadow, &cb0ShadowVirtualPtr);

	// DescriptorHeap
	const DescriptorHeap descriptorHeapBasicShadow
		= D3D12Utils::InitDescriptorHeapBasic(device, { cb0Shadow }, { {shadowGraphicsBuffer, shadowTextureMetadata} });

	const ViewportScissorRect viewportScissorRectShadow = ViewportScissorRect::CreateFullSized(ShadowRTSize);

#pragma endregion

	TerrainRenderer terrainRenderer = TerrainRenderer(
		device, commandList, commandQueue, commandAllocator, WindowSize,
		{ shadowGraphicsBuffer, shadowTextureMetadata }
	);
	terrainRenderer.OnPlayerCameraMatrixChanged(playerController.CalculateVPMatrix());
	terrainRenderer.OnSunCameraMatrixChanged(sunCamera.CalculateVPMatrix());
	terrainRenderer.OnSunCameraParameterChanged(SunCamera::Direction, SunCamera::ShadowColor);

	const std::unique_ptr<AOffscreenRenderer> postProcessRenderer =
		std::make_unique<PostProcessRenderer>(device, commandList, commandQueue, commandAllocator, WindowSize);

	// UIテキストのデータはゲーム内で変更されるため、const には出来ない (このオブジェクトが内部で保持している)
	std::unique_ptr<AOffscreenRenderer> textRenderer =
		std::make_unique<TextRenderer>(device, commandList, commandQueue, commandAllocator, WindowSize);

	const std::unique_ptr<AOffscreenRenderer> pointerImageRenderer =
		std::make_unique<PointerImageRenderer>(device, commandList, commandQueue, commandAllocator, WindowSize);

	ItemSlotManager itemSlotManager = ItemSlotManager(device, commandList, commandQueue, commandAllocator, WindowSize);

	// [ms] 単位でのフレーム時間統計
	DebugFrameTimeStats frameTimeStatsPreFrame = DebugFrameTimeStats(16);
	DebugFrameTimeStats frameTimeStatsPostFrame = DebugFrameTimeStats(16);
	DebugFrameTimeStats frameTimeStatsCPU = DebugFrameTimeStats(16);
	DebugFrameTimeStats frameTimeStatsGPU = DebugFrameTimeStats(16);



	while (true)
	{
		const double timeBeforeFrame = WindowHelper::GetTime();

		if (!WindowHelper::OnBeginFrame(hwnd))
			break;

		const double timeBeforeCPU = WindowHelper::GetTime();
		frameTimeStatsPreFrame.Record(timeBeforeCPU - timeBeforeFrame);

		// Escape でゲーム終了
		if (InputHelper::GetKeyInfo(Key::Escape).pressedNow)
			break;

		// プレイヤーの挙動
		const PlayerController::Inputs playerInputs =
		{
			.move = InputHelper::GetAsAxis2D(Key::W, Key::S, Key::A, Key::D),
			.look = InputHelper::GetMouseDelta(),
			.dashPressed = InputHelper::GetKeyInfo(Key::LShift).pressed,
			.jumpPressed = InputHelper::GetKeyInfo(Key::Space).pressed,
		};
		playerController.OnEveryFrame(chunksManager.GetChunks(), playerInputs, WindowHelper::GetDeltaSeconds());
		terrainRenderer.OnPlayerCameraMatrixChanged(playerController.CalculateVPMatrix());

		// アイテムスロットの選択変更
		const ItemSlotManager::SelectInputs itemSlotSelectInputs =
		{
			.select1 = InputHelper::GetKeyInfo(Key::N1).pressedNow,
			.select2 = InputHelper::GetKeyInfo(Key::N2).pressedNow,
			.select3 = InputHelper::GetKeyInfo(Key::N3).pressedNow,
			.select4 = InputHelper::GetKeyInfo(Key::N4).pressedNow,
			.select5 = InputHelper::GetKeyInfo(Key::N5).pressedNow,
			.select6 = InputHelper::GetKeyInfo(Key::N6).pressedNow,

			.selectLeft = InputHelper::GetMouseWheelDelta() > 0.1f,   // マウスホイールが上に回った瞬間だけ true
			.selectRight = InputHelper::GetMouseWheelDelta() < -0.1f, // マウスホイールが下に回った瞬間だけ true
		};
		itemSlotManager.UpdateSelectingSlotByInput(itemSlotSelectInputs, device, commandList, commandQueue, commandAllocator);

		// 見ているブロック・フェースを取得
		const auto [lookingBlockPosition, lookingBlockFaceNormal] = playerController.PickLookingBlock(chunksManager.GetChunks());

		// ブロックを選択していない
		if (lookingBlockFaceNormal == Lattice3::Zero())
		{
			if (auto* cb1VirtualPtr = terrainRenderer.GetCB1VirtualPtr())
			{
				cb1VirtualPtr->IsSelectingBlock = 0;
				cb1VirtualPtr->SelectingBlockWorldPosition = Lattice3::Zero();
			}
		}
		// ブロックを選択している
		else
		{
			if (auto* cb1VirtualPtr = terrainRenderer.GetCB1VirtualPtr())
			{
				cb1VirtualPtr->IsSelectingBlock = 1;
				cb1VirtualPtr->SelectingBlockWorldPosition = lookingBlockPosition;
			}

			// マウスホイールが押されたら、アイテムスロット内に見ているブロックがあるか調べる
			// もしあったら、そのスロットを直ちに選択する
			if (InputHelper::GetKeyInfo(Key::MMouse).pressedNow)
			{
				const Block lookingBlock = chunksManager.GetBlock(lookingBlockPosition);
				for (int i = 0; i < ItemSlotManager::SlotCount; ++i)
				{
					if (ItemSlotManager::SlotItems[i] == lookingBlock)
					{
						itemSlotManager.UpdateSelectingSlot(i, device, commandList, commandQueue, commandAllocator);
						break;
					}
				}
			}

			static Timer mineCdTimer = Timer(PlayerController::MineCooldownSeconds);
			static Timer placeCdTimer = Timer(PlayerController::PlaceCooldownSeconds);

			mineCdTimer.OnEveryFrame(WindowHelper::GetDeltaSeconds());
			placeCdTimer.OnEveryFrame(WindowHelper::GetDeltaSeconds());

			// ブロックを採掘する
			if (mineCdTimer.IsFinished() && InputHelper::GetKeyInfo(Key::LMouse).pressed)
			{
				mineCdTimer.Reset();
				const bool _ = playerController.TryMineBlock(
					chunksManager, lookingBlockPosition, device);
			}

			// ブロックを設置する
			if (placeCdTimer.IsFinished() && InputHelper::GetKeyInfo(Key::RMouse).pressed)
			{
				placeCdTimer.Reset();

				const Block placeBlock = ItemSlotManager::SlotItems[itemSlotManager.GetSelectingIndex()];
				if (placeBlock != Block::Invalid)
					const bool _ = playerController.TryPlaceBlock(
						chunksManager, lookingBlockPosition + lookingBlockFaceNormal, placeBlock, device);
			}

			// マウスボタンが離されたら、クールタイムを即リセットする
			if (InputHelper::GetKeyInfo(Key::LMouse).releasedNow)
				mineCdTimer.CountToFinishImmediately();
			if (InputHelper::GetKeyInfo(Key::RMouse).releasedNow)
				placeCdTimer.CountToFinishImmediately();
		}

		// プレイヤーの存在チャンクが変化したなら、描画チャンクを更新する
		playerExistingChunkIndex = Chunk::GetIndex(playerController.GetFootBlockPosition());
		if (playerExistingChunkIndex.DropDirty())
		{
			chunksManager.UpdateDrawChunks(playerExistingChunkIndex.GetValue(), true, device);
		}

		// デバッグテキスト
		{
			static DebugTextDisplayer debugTextDisplayer{};

			const DebugTextDisplayer::DebugFrameTimeStatsBreakdown frameTimeStatsBreakdown =
			{
				.preFrame = frameTimeStatsPreFrame,
				.cpu = frameTimeStatsCPU,
				.gpu = frameTimeStatsGPU,
				.postFrame = frameTimeStatsPostFrame,
			};

			debugTextDisplayer.UpdateData(
				*dynamic_cast<TextRenderer*>(textRenderer.get()),
				device, commandList, commandQueue, commandAllocator,
				playerController, chunksManager,
				frameTimeStatsBreakdown,
				{
					.isLooking = terrainRenderer.GetCB1VirtualPtr()->IsSelectingBlock == 1,
					.lookingBlockWorldPosition = terrainRenderer.GetCB1VirtualPtr()->SelectingBlockWorldPosition,
					.lookingBlockFaceNormal = lookingBlockFaceNormal
				}
			);
		}

		// 太陽カメラの位置を、プレイヤーの頭上らへんにする
		{
			sunCamera.LookAtPlayer(playerController.GetFootPosition());

			terrainRenderer.OnSunCameraMatrixChanged(sunCamera.CalculateVPMatrix());
			cb0ShadowVirtualPtr->Matrix_MVP = sunCamera.CalculateVPMatrix() * TerrainRenderer::Transform.CalculateModelMatrix();
		}

		const double timeAfterCPU = WindowHelper::GetTime();
		frameTimeStatsCPU.Record(timeAfterCPU - timeBeforeCPU);



		const int currentBackRTIndex = D3D12Helper::GetCurrentBackRTIndex(swapChain);
		const GraphicsBuffer currentBackRT = rtGetter(currentBackRTIndex);
		const DescriptorHandleAtCPU currentBackRTV = rtvGetter(currentBackRTIndex);
		if (!currentBackRT)
			ShowError(L"現在のバックレンダーターゲットの取得に失敗しました");

		const auto& packedDrawVBVs = chunksManager.PackDrawVBVs();
		const auto& packedDrawIBVs = chunksManager.PackDrawIBVs();
		const auto& packedDrawMeshIndicesCounts = chunksManager.PackDrawMeshIndicesCounts();

		// 影のデプス書き込み
		if (terrainRenderer.GetCB1VirtualPtr()->CastShadow == 1)
		{
			D3D12Utils::Draw(
				commandList, commandQueue, commandAllocator, device,
				rootSignatureShadow, graphicsPipelineStateShadow, shadowGraphicsBuffer,
				rtvShadow, dsvShadow, descriptorHeapBasicShadow, packedDrawVBVs, packedDrawIBVs,
				GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
				viewportScissorRectShadow, PrimitiveTopology::TriangleList, Color(DepthBufferClearValue, 0, 0, 0), DepthBufferClearValue,
				packedDrawMeshIndicesCounts
			);
		}
		// メインレンダリング
		terrainRenderer.Draw(
			commandList, commandQueue, commandAllocator, device,
			postProcessRenderer->GetRT(), postProcessRenderer->GetRTV(), viewportScissorRect,
			packedDrawVBVs, packedDrawIBVs, packedDrawMeshIndicesCounts
		);
		// オフスクリーンレンダリング
		{
			std::vector<const AOffscreenRenderer*> renderers = {};

			renderers.push_back(postProcessRenderer.get());  // ポストプロセス
			renderers.push_back(textRenderer.get());         // テキスト描画
			renderers.push_back(pointerImageRenderer.get()); // ポインター画像描画

			// アイテムスロット画像描画
			const auto itemSlotRenderers = itemSlotManager.PackRenderers();
			renderers.insert(renderers.end(), itemSlotRenderers.begin(), itemSlotRenderers.end());

			AOffscreenRenderer::DrawInOrder(
				commandList, commandQueue, commandAllocator, device,
				currentBackRT, currentBackRTV, viewportScissorRect,
				renderers
			);
		}

		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");

		const double timeAfterGPU = WindowHelper::GetTime();
		frameTimeStatsGPU.Record(timeAfterGPU - timeAfterCPU);

		WindowHelper::OnEndFrame();

		const double timeAfterFrame = WindowHelper::GetTime();
		frameTimeStatsPostFrame.Record(timeAfterFrame - timeAfterGPU);
	}

	return 0;
}
