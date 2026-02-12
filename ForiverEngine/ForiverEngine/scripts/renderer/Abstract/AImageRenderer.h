#pragma once

#include "scripts/renderer/IncludeInternal.h"
#include "scripts/renderer/Context/Include.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>画像のレンダラー</para>
	/// <para>Quadメッシュを作成し、画像テクスチャを貼り付ける</para>
	/// <para>動的に位置やサイズを変更、などは未対応</para>
	/// </summary>
	class AImageRenderer
	{
	private:
		// b0
		struct alignas(256) CBData0
		{
			// メッシュの座標は、スクリーン座標系 (NDC) で与えられることを想定

			std::uint32_t IsDrawEnabled; // 描画するかどうか (0: しない, 1: する)
		};

	public:
		virtual ~AImageRenderer() = default;

		/// <summary>
		/// <para>初期化. 描画に必要なオブジェクトを作成する</para>
		/// <para>使い方の例としては、基底クラスのコンストラクタの最後でこの Init() を呼び出すとか</para>
		/// <para>位置・サイズは、スクリーンのピクセル単位で指定する</para>
		/// </summary>
		void Init(
			const RenderContext& renderContext, const Lattice2& windowSize,
			const std::string& imageFilePath,
			const Lattice2& position, const Lattice2& size, bool initDrawEnabled = true
		)
		{
			// RootSignature, PipelineState
			const RootParameter rootParameter = RootParameter::CreateBasic(1, 1);
			const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
			const auto [shaderVS, shaderPS] = D3D12Utils::LoadCso(D3D12Utils::GetShaderFilePath("QuadImage"));
			std::tie(rootSignature, pipelineState) = D3D12Utils::CreateRootSignatureAndGraphicsPipelineState(
				renderContext.device,
				rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, true);

			dsv = D3D12Utils::InitDSV(renderContext.device, windowSize);

			// メッシュ
			MeshQuad mesh = MeshQuad::CreateFullSized();
			for (auto& vertex : mesh.vertices)
			{
				// 指定されたサイズに設定
				// スクリーン全体サイズだと座標は -1 ~ 1 になることに注意
				vertex.pos.x *= 1.0f * size.x / windowSize.x;
				vertex.pos.y *= 1.0f * size.y / windowSize.y;

				// 指定された位置に設定
				// windowSize -> [0.0f,1.0f] -> [-1.0f,1.0f]
				// Y 軸は上下逆転に注意
				vertex.pos.x += 2.0f * position.x / windowSize.x - 1.0f;
				vertex.pos.y += -2.0f * position.y / windowSize.y + 1.0f;
			}
			// VBV, IBV
			auto [vbv, ibv] = mesh.CreateRenderViews(renderContext.device);
			// 頂点数
			const int indexCount = static_cast<int>(mesh.indices.size());
			renderMeshContext = RenderMeshContext
			{
				.vbvList = { std::move(vbv) },
				.ibvList = { std::move(ibv) },
				.indexCountList = { indexCount },
			};

			// b0
			CBData0 cbData0 =
			{
				.IsDrawEnabled = (initDrawEnabled ? 1u : 0u),
			};
			const GraphicsBuffer cb0 = D3D12Utils::InitCB(renderContext.device, cbData0, &cb0VirtualPtr);

			// s0
			const Texture sr0Metadata = D3D12Utils::LoadTexture({ imageFilePath });
			const GraphicsBuffer sr0 = D3D12Utils::InitSR(
				renderContext.device, renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, sr0Metadata
			);

			// DescriptorHeap
			descriptorHeapBasic = D3D12Utils::InitDescriptorHeapBasic(renderContext.device, { cb0 }, { { sr0, sr0Metadata } });
		}

		// Init 後に使うこと!
		bool GetDrawEnabled() const
		{
			return (cb0VirtualPtr->IsDrawEnabled == 0u) ? false : true;
		}
		// Init 後に使うこと!
		void SetDrawEnabled(bool enabled)
		{
			cb0VirtualPtr->IsDrawEnabled = (enabled ? 1u : 0u);
		}

		/// <summary>
		/// <para>ドローコール</para>
		/// </summary>
		void Draw(
			const RenderContext& renderContext,
			const RenderTargetContext& renderTargetContext
		) const
		{
			D3D12Utils::Draw(
				renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, renderContext.device,
				rootSignature, pipelineState, renderTargetContext.rt,
				renderTargetContext.rtv, dsv, descriptorHeapBasic, renderMeshContext.vbvList, renderMeshContext.ibvList,
				GraphicsBufferState::Present, GraphicsBufferState::RenderTarget,
				renderTargetContext.viewportScissorRect, PrimitiveTopology::TriangleList, Color::Transparent(), DepthBufferClearValue,
				renderMeshContext.indexCountList
			);
		}

	private:
		RootSignature rootSignature;
		PipelineState pipelineState;
		DescriptorHandleAtCPU dsv;
		DescriptorHeap descriptorHeapBasic;
		RenderMeshContext renderMeshContext;

		CBData0* cb0VirtualPtr = nullptr;
	};
}
