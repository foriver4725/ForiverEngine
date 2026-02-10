#pragma once

#include "scripts/gameFlow/IncludeInternal.h"

namespace ForiverEngine
{
	/// <summary>
	/// 地形レンダラー
	/// </summary>
	class TerrainRenderer
	{
	public:
		static constexpr Color SkyColor = Color::CreateFromUint8(60, 150, 210); // 背景色 (空色)
		// 地形のメッシュデータはワールド座標に変換済みなので、Transform は単位行列で良い
		static constexpr Transform Transform = Transform::Identity();

		// ブロックのテクスチャ画像ファイルパス
		// ブロック種類の列挙型と同じ順番にすること!!
		inline static const std::vector<std::string> BlockTextureFilePaths =
		{
			"assets/textures/block/air_invalid.png",
			"assets/textures/block/grass_stone.png",
			"assets/textures/block/dirt_sand.png",
		};

	private:
		// b0
		struct alignas(256) CBData0
		{
			Matrix4x4 Matrix_MVP;                 // MVP
			Matrix4x4 Matrix_M_IT;                // M の逆→転置行列 (法線用だけど、ぶっちゃけ今の設計的に、より計算コストがかかっている)
		};
	public:
		// b1
		struct alignas(256) CBData1
		{
			Lattice3 SelectingBlockWorldPosition; // 選択中のブロック位置
			std::uint32_t IsSelectingBlock;       // ブロックを選択中かどうか (bool 型として扱う)
			Color SelectColor;                    // 選択中のブロックの乗算色 (a でブレンド率を指定)

			Vector3 DirectionalLightDirection;    // 太陽光の向き (正規化済み)
			float Pad0;
			Color DirectionalLightColor;          // 太陽光の色 (a は使わない)
			Color AmbientLightColor;              // 環境光の色 (a は使わない)
		};

	public:
		virtual ~TerrainRenderer() = default;

		/// <summary>
		/// <para>コンストラクタ. 描画オブジェクトを初期化する</para>
		/// <para>b0,b1 を使用. b0 は行列オブジェクトで、なるべく隠蔽する. b1 はその他のデータで、直接外部公開する</para>
		/// <para>t0 を使用. t0 はブロックのテクスチャ配列</para>
		/// <para>RTV は作らない (多分 SwapChain に関連するので、面倒くさい)</para>
		/// <para>仮値を設定している所がある. 必ず直後に初期値を設定すること!</para>
		/// </summary>
		explicit TerrainRenderer(const RenderContext& renderContext, const Lattice2& windowSize)
			: Matrix_M_Cached(Transform.CalculateModelMatrix())
		{
			// RootSignature, PipelineState
			const RootParameter rootParameter = RootParameter::CreateBasic(2, 1);
			const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
			const auto [shaderVS, shaderPS] = D3D12Utils::LoadCso(D3D12Utils::GetShaderFilePath("Basic"));
			std::tie(rootSignature, pipelineState) = D3D12Utils::CreateRootSignatureAndGraphicsPipelineState(
				renderContext.device,
				rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayouts, FillMode::Solid, CullMode::None, true);

			// RTV は作らない!
			dsv = D3D12Utils::InitDSV(renderContext.device, windowSize);

			// b0, b1
			CBData0 cbData0 =
			{
				.Matrix_MVP = Matrix4x4::Identity(), // 仮値. 直後に初期値を設定してもらう前提
				.Matrix_M_IT = Transform.CalculateModelMatrixInversed().Transposed(),
			};
			CBData1 cbData1 =
			{
				.SelectingBlockWorldPosition = Lattice3::Zero(),
				.IsSelectingBlock = 0,
				.SelectColor = Color::CreateFromUint8(255, 255, 0, 48),

				.DirectionalLightDirection = Vector3(1.0f, -1.0f, 1.0f).Normed(),
				.DirectionalLightColor = Color::White() * 1.2f,
				.AmbientLightColor = Color::White() * 0.5f,
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(renderContext.device, cbData0, &cb0VirtualPtr);
			const GraphicsBuffer cb1 = D3D12Utils::InitCB(renderContext.device, cbData1, &cb1VirtualPtr);

			// t0
			const Texture textureArray = D3D12Utils::LoadTexture(BlockTextureFilePaths);
			const auto sr = D3D12Utils::InitSR(
				renderContext.device, renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, textureArray);

			// DescriptorHeap
			descriptorHeapBasic = D3D12Utils::InitDescriptorHeapBasic(renderContext.device, { cb0, cb1 }, { { sr, textureArray } });
		}

		/// <summary>
		/// <para>外部依存の処理. 忘れずに呼んでね</para>
		/// </summary>
		void OnPlayerCameraMatrixChanged(const Matrix4x4& playerCameraVPMatrix)
		{
			cb0VirtualPtr->Matrix_MVP = playerCameraVPMatrix * Matrix_M_Cached;
		}

		CBData1* GetCB1VirtualPtr()
		{
			return cb1VirtualPtr;
		}

		/// <summary>
		/// <para>ドローコール</para>
		/// <para>指定された RT に対してレンダリングする</para>
		/// <para>直前のレンダリングがどうであるかは気にしない (多分 SwapChain に関連するので、面倒くさい)</para>
		/// <para>チャンクデータから VBV,IBV,頂点数 を算出して渡すこと</para>
		/// </summary>
		void Draw(
			const RenderContext& renderContext,
			const RenderTargetContext& renderTargetContext,
			const RenderMeshContext& renderMeshContext
		) const
		{
			D3D12Utils::Draw(
				renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, renderContext.device,
				rootSignature, pipelineState, renderTargetContext.rt,
				renderTargetContext.rtv, dsv, descriptorHeapBasic, renderMeshContext.vbvList, renderMeshContext.ibvList,
				GraphicsBufferState::PixelShaderResource, GraphicsBufferState::RenderTarget,
				renderTargetContext.viewportScissorRect, PrimitiveTopology::TriangleList, SkyColor, DepthBufferClearValue,
				renderMeshContext.indexCountList
			);
		}

	private:
		RootSignature rootSignature;
		PipelineState pipelineState;
		DescriptorHandleAtCPU dsv;
		DescriptorHeap descriptorHeapBasic;

		// 何か所かで使うので、キャッシュしておく (自身の Transform が変化しないので、一度だけ計算すれば良い)
		Matrix4x4 Matrix_M_Cached;

		CBData0* cb0VirtualPtr = nullptr;
		CBData1* cb1VirtualPtr = nullptr;
	};
}
