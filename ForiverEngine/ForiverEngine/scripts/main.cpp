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

#ifdef _DEBUG
	// テストコード実行
#if 0
	Test::PlayerControl::RunAll();

	ShowError(L"全てのテストに成功しました");
	return 0;
#endif

	// シェーダーのコンパイル (開発中のみ)
#if 0
	const bool success = D3D12Utils::IDE_CompileShader(
		{
			//"Basic.hlsl",
			//"PP.hlsl",
			//"Text.hlsl",
			//"Image.hlsl",
		});

	ShowError(success ? L"シェーダーのコンパイルに成功しました" : L"シェーダーのコンパイルに失敗しました");
	return 0;
#endif

	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	WindowHelper::SetTargetFps(60);
	WindowHelper::SetCursorEnabled(false);

	constexpr std::uint32_t RandomSeed = 0x12345678;
	Random::SetSeed(RandomSeed);

	const auto [factory, device, commandAllocator, commandList, commandQueue]
		= D3D12Utils::CreateStandardObjects();

	const SwapChain swapChain = D3D12Helper::CreateSwapChain(factory, commandQueue, hwnd, WindowSize);
	if (!swapChain)
		ShowError(L"SwapChain の作成に失敗しました");
	const auto [rtGetter, rtvGetter] = D3D12Utils::InitRTV(device, swapChain, Format::RGBA_U8_01);

	const ViewportScissorRect viewportScissorRect = ViewportScissorRect::CreateFullSized(WindowSize);



	// プレイヤーが存在するチャンクのインデックス
	constexpr Lattice2 playerInitChunkIndex = Lattice2(Chunk::Count / 2, Chunk::Count / 2); // 初期スポーン地点は、ワールドのど真ん中
	TrackedValue<Lattice2> playerExistingChunkIndex = TrackedValue(playerInitChunkIndex);

	// 地形データ
	ChunksManager chunksManager = ChunksManager(playerExistingChunkIndex.GetValue());
	chunksManager.UpdateDrawChunks(playerExistingChunkIndex.GetValue(), false, device); // 初回作成

	// プレイヤーコントローラー
	PlayerController playerController = PlayerController(WindowSize, playerExistingChunkIndex.GetValue(), chunksManager.GetChunks());

	TerrainRenderer terrainRenderer = TerrainRenderer(device, commandList, commandQueue, commandAllocator, WindowSize);
	terrainRenderer.OnPlayerCameraMatrixChanged(playerController.CalculateVPMatrix());

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

		// デバッグテキスト (F1 押下で表示切替)
		{
			static DebugTextDisplayer debugTextDisplayer{};
			static bool fold = true; // true = 折りたたみ表示, false = 展開表示

			if (InputHelper::GetKeyInfo(Key::F1).pressedNow)
				fold = !fold;

			TextRenderer& textRendererRef = *dynamic_cast<TextRenderer*>(textRenderer.get());
			if (fold)
			{
				debugTextDisplayer.UpdateDataAsFold(textRendererRef, device, commandList, commandQueue, commandAllocator);
			}
			else
			{
				const DebugTextDisplayer::DebugFrameTimeStatsBreakdown frameTimeStatsBreakdown =
				{
					.preFrame = frameTimeStatsPreFrame,
					.cpu = frameTimeStatsCPU,
					.gpu = frameTimeStatsGPU,
					.postFrame = frameTimeStatsPostFrame,
				};

				const DebugText::LookingBlockInfo lookingBlockInfo =
				{
					.isLooking = terrainRenderer.GetCB1VirtualPtr()->IsSelectingBlock == 1,
					.lookingBlockWorldPosition = terrainRenderer.GetCB1VirtualPtr()->SelectingBlockWorldPosition,
					.lookingBlockFaceNormal = lookingBlockFaceNormal,
				};

				debugTextDisplayer.UpdateDataAsUnfold(
					textRendererRef, device, commandList, commandQueue, commandAllocator,
					playerController, chunksManager, frameTimeStatsBreakdown, lookingBlockInfo
				);
			}
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
