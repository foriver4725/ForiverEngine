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
		= D3D12Utils::CreateStandardObjects();



	constexpr Lattice2 playerInitChunkIndex = Lattice2(Chunk::Count / 2, Chunk::Count / 2); // 初期スポーン地点は、ワールドのど真ん中
	TrackedValue playerExistingChunkIndex = TrackedValue(playerInitChunkIndex);

	// 地形データ
	ChunksManager chunksManager = ChunksManager(playerExistingChunkIndex.GetValue());
	chunksManager.UpdateDrawChunks(playerExistingChunkIndex.GetValue(), false, device); // 初回作成

	constexpr Transform terrainTransform = Transform::Identity();

	PlayerController playerController = PlayerController(WindowSize, playerExistingChunkIndex.GetValue(), chunksManager.GetChunks());

	SunCamera sunCamera = SunCamera();
	sunCamera.LookAtPlayer(playerController.GetFootPosition());

#pragma region Shadow

	constexpr Lattice2 ShadowRTSize = Lattice2(1024, 1024);

	const Texture shadowTextureMetadata = Texture::CreateManually({}, ShadowRTSize, Format::R_F32);
	const GraphicsBuffer shadowGraphicsBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, shadowTextureMetadata,
		GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color(DepthBufferClearValue, 0, 0, 0));

	const RootParameter rootParameterShadow = RootParameter::CreateBasic(1, 1);
	const SamplerConfig samplerConfigShadow = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVSShadow, shaderPSShadow] = D3D12Utils::CompileShader_VS_PS("./shaders/ShadowDepthWrite.hlsl");
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
		.Matrix_MVP = sunCamera.CalculateVPMatrix() * terrainTransform.CalculateModelMatrix(),
	};
	CBData0Shadow* cb0ShadowVirtualPtr = nullptr;
	const GraphicsBuffer cb0Shadow = D3D12Utils::InitCB(device, cbData0Shadow, &cb0ShadowVirtualPtr);

	// DescriptorHeap
	const DescriptorHeap descriptorHeapBasicShadow
		= D3D12Utils::InitDescriptorHeapBasic(device, { cb0Shadow }, { {shadowGraphicsBuffer, shadowTextureMetadata} });

	const ViewportScissorRect viewportScissorRectShadow = ViewportScissorRect::CreateFullSized(ShadowRTSize);

#pragma endregion

#pragma region MainRender

	const RootParameter rootParameter = RootParameter::CreateBasic(2, 2);
	const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
	const auto [shaderVS, shaderPS] = D3D12Utils::CompileShader_VS_PS("./shaders/Basic.hlsl");
	const auto [rootSignature, graphicsPipelineState]
		= D3D12Utils::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayouts, FillMode::Solid, CullMode::None, true);

	const SwapChain swapChain = D3D12Helper::CreateSwapChain(factory, commandQueue, hwnd, WindowSize);
	if (!swapChain)
		ShowError(L"SwapChain の作成に失敗しました");
	const auto [rtGetter, rtvGetter] = D3D12Utils::InitRTV(device, swapChain, Format::RGBA_U8_01);
	const DescriptorHandleAtCPU dsv = D3D12Utils::InitDSV(device, WindowSize);

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
		.Matrix_MVP = playerController.CalculateVPMatrix() * terrainTransform.CalculateModelMatrix(),
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
	CBData0* cb0VirtualPtr = nullptr;
	CBData1* cb1VirtualPtr = nullptr;
	const GraphicsBuffer cb0 = D3D12Utils::InitCB(device, cbData0, &cb0VirtualPtr);
	const GraphicsBuffer cb1 = D3D12Utils::InitCB(device, cbData1, &cb1VirtualPtr);

	// SRV 用バッファ
	const Texture textureArray = D3D12Utils::LoadTexture({
			"assets/textures/air_invalid.png",
			"assets/textures/grass_stone.png",
			"assets/textures/dirt_sand.png",
		});
	const auto sr = D3D12Utils::InitSR(device, commandList, commandQueue, commandAllocator, textureArray);

	// DescriptorHeap に登録
	DescriptorHeap descriptorHeapBasic = D3D12Utils::InitDescriptorHeapBasic(
		device, { cb0, cb1 }, { {sr, textureArray}, {shadowGraphicsBuffer, shadowTextureMetadata} });

	const ViewportScissorRect viewportScissorRect = ViewportScissorRect::CreateFullSized(WindowSize);

#pragma	endregion

	const PostProcessRenderer postProcessRenderer{
		device, commandList, commandQueue, commandAllocator,
		WindowSize
	};

	// UIテキストのデータはゲーム内で変更されるため、const には出来ない (このオブジェクトが内部で保持している)
	TextRenderer textRenderer{
	   device, commandList, commandQueue, commandAllocator,
	   WindowSize
	};



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
			.look = InputHelper::GetMouseDelta(),
			.dashPressed = InputHelper::GetKeyInfo(Key::LShift).pressed,
			.jumpPressed = InputHelper::GetKeyInfo(Key::Space).pressed,
		};
		playerController.OnEveryFrame(chunksManager.GetChunks(), playerInputs, WindowHelper::GetDeltaSeconds());
		cb0VirtualPtr->Matrix_MVP = playerController.CalculateVPMatrix() * terrainTransform.CalculateModelMatrix();

		// 見ているブロック・フェースを取得
		const auto [lookingBlockPosition, lookingBlockFaceNormal] = playerController.PickLookingBlock(chunksManager.GetChunks());

		// ブロックを選択する
		{
			if (lookingBlockFaceNormal == Lattice3::Zero())
			{
				cb1VirtualPtr->IsSelectingBlock = 0;
				cb1VirtualPtr->SelectingBlockWorldPosition = Lattice3::Zero();
			}
			else
			{
				cb1VirtualPtr->IsSelectingBlock = 1;
				cb1VirtualPtr->SelectingBlockWorldPosition = lookingBlockPosition;
			}
		}

		{
			// ブロックを採掘する
			{
				static float mineCooldownTimer = 0.0f;

				if (mineCooldownTimer <= 0.0f)
				{
					if (InputHelper::GetKeyInfo(Key::LMouse).pressed)
					{
						if (cb1VirtualPtr->IsSelectingBlock == 1)
						{
							mineCooldownTimer = PlayerController::MineCooldownSeconds;
							const auto _ = playerController.TryMineBlock(
								chunksManager, cb1VirtualPtr->SelectingBlockWorldPosition, device);
						}
					}
				}

				mineCooldownTimer -= WindowHelper::GetDeltaSeconds();
				if (mineCooldownTimer < 0.0f)
					mineCooldownTimer = 0.0f;
			}

			// ブロックを設置する
			{
				static float placeCooldownTimer = 0.0f;

				if (placeCooldownTimer <= 0.0f)
				{
					if (InputHelper::GetKeyInfo(Key::RMouse).pressed)
					{
						if (cb1VirtualPtr->IsSelectingBlock == 1)
						{
							placeCooldownTimer = PlayerController::PlaceCooldownSeconds;
							const auto _ = playerController.TryPlaceBlock(
								chunksManager, lookingBlockPosition + lookingBlockFaceNormal, device);
						}
					}
				}

				placeCooldownTimer -= WindowHelper::GetDeltaSeconds();
				if (placeCooldownTimer < 0.0f)
					placeCooldownTimer = 0.0f;
			}

			// プレイヤーの存在チャンクが変化したなら、描画チャンクを更新する
			playerExistingChunkIndex = Chunk::GetIndex(playerController.GetFootBlockPosition());
			if (playerExistingChunkIndex.DropDirty())
			{
				chunksManager.UpdateDrawChunks(playerExistingChunkIndex.GetValue(), true, device);
			}
		}

		// デバッグテキスト
		{
			static DebugFrameTimeStats frameTimeStats = DebugFrameTimeStats(16);
			frameTimeStats.Record(WindowHelper::GetDeltaMilliseconds());

			static DebugTextDisplayer debugTextDisplayer{};
			debugTextDisplayer.UpdateData(
				textRenderer,
				device, commandList, commandQueue, commandAllocator,
				playerController, chunksManager,
				frameTimeStats,
				{
					.isLooking = cb1VirtualPtr->IsSelectingBlock == 1,
					.lookingBlockWorldPosition = cb1VirtualPtr->SelectingBlockWorldPosition,
					.lookingBlockFaceNormal = lookingBlockFaceNormal
				}
			);
		}

		// 太陽カメラの位置を、プレイヤーの頭上らへんにする
		{
			sunCamera.LookAtPlayer(playerController.GetFootPosition());

			cb0VirtualPtr->DirectionalLight_Matrix_VP = sunCamera.CalculateVPMatrix();
			cb0ShadowVirtualPtr->Matrix_MVP = sunCamera.CalculateVPMatrix() * terrainTransform.CalculateModelMatrix();
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
		if (cb1VirtualPtr->CastShadow == 1)
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
		D3D12Utils::Draw(
			commandList, commandQueue, commandAllocator, device,
			rootSignature, graphicsPipelineState, postProcessRenderer.GetRT(),
			postProcessRenderer.GetRTV(), dsv, descriptorHeapBasic, packedDrawVBVs, packedDrawIBVs,
			GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
			viewportScissorRect, PrimitiveTopology::TriangleList, RTClearColor, DepthBufferClearValue,
			packedDrawMeshIndicesCounts
		);
		// ポストプロセス
		postProcessRenderer.Draw(
			commandList, commandQueue, commandAllocator, device,
			textRenderer.GetRT(), textRenderer.GetRTV(), viewportScissorRect
		);
		// テキスト描画
		textRenderer.Draw(
			commandList, commandQueue, commandAllocator, device,
			currentBackRT, currentBackRTV, viewportScissorRect
		);
		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");

		WindowHelper::OnEndFrame();
	}

	return 0;
}
