#pragma once

#include "scripts/renderer/IncludeInternal.h"
#include "scripts/renderer/Context/Include.h"

namespace ForiverEngine
{
	/// <summary>
	/// <para>オフスクリーンレンダラー (抽象クラス)</para>
	/// <para>RT を SR としても使うことができる</para>
	/// </summary>
	class AOffscreenRenderer
	{
	public:
		virtual ~AOffscreenRenderer() = default;

		/// <summary>
		/// <para>初期化. 描画に必要なオブジェクトを作成する</para>
		/// <para>使い方の例としては、基底クラスのコンストラクタで CB, SR 群などを作成し、そのコンストラクタの最後でこの Init() を呼び出すとか</para>
		/// </summary>
		void Init(
			const RenderContext& renderContext, const Lattice2& windowSize,
			const std::vector<GraphicsBuffer>& cbs,                     // 順に b0~ にバインドされる
			const std::vector<std::pair<GraphicsBuffer, Texture>>& srs, // 順に t1~ にバインドされる (t0 は RT/SR 用)
			const std::string& shaderFilePath
		)
		{
			cbCount = static_cast<int>(cbs.size());
			srCount = static_cast<int>(srs.size());

			// RT,SR となるバッファを作成
			const Texture rtMetadata = Texture({}, Lattice3(windowSize, 1), Format::RGBA_U8_01);
			rt = D3D12Helper::CreateGraphicsBufferTexture2D(renderContext.device, rtMetadata,
				GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color::Transparent());

			// RootSignature, PipelineState
			const RootParameter rootParameter = RootParameter::CreateBasic(cbCount, srCount + 1);
			const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
			const auto [shaderVS, shaderPS] = D3D12Utils::LoadCso(shaderFilePath);
			std::tie(rootSignature, pipelineState) = D3D12Utils::CreateRootSignatureAndGraphicsPipelineState(
				renderContext.device,
				rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, false);

			// RTVのみ作成
			rtv = D3D12Utils::InitRTV(renderContext.device, rt, Format::RGBA_U8_01);
			dsv_Dummy = DescriptorHandleAtCPU{ .ptr = NULL };

			// メッシュ
			const MeshQuad mesh = MeshQuad::CreateFullSized();
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

			// SR 群の最初に RT/SR を登録する (t0)
			std::vector<std::pair<GraphicsBuffer, Texture>> srsWithRT;
			srsWithRT.reserve(srCount + 1);
			srsWithRT.emplace_back(rt, rtMetadata);
			srsWithRT.insert(srsWithRT.end(), srs.begin(), srs.end());

			// DescriptorHeap
			descriptorHeapBasic = D3D12Utils::InitDescriptorHeapBasic(renderContext.device, cbs, srsWithRT);
		}

		const GraphicsBuffer& GetRT() const
		{
			return rt;
		}
		const DescriptorHandleAtCPU& GetRTV() const
		{
			return rtv;
		}

		/// <summary>
		/// <para>指定したテクスチャからバッファを作成し、GPU に再アップロードする</para>
		/// <para>shaderRegister で指定した t レジスタにバインドされる</para>
		/// </summary>
		void ReUploadTexture(const RenderContext& renderContext, const Texture& texture, ShaderRegister shaderRegister)
		{
			const GraphicsBuffer sr = D3D12Utils::InitSR(
				renderContext.device, renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, texture);
			D3D12Helper::CreateSRVAndRegistToDescriptorHeap(renderContext.device, descriptorHeapBasic, sr,
				static_cast<int>(shaderRegister) + cbCount, texture);
		}

		/// <summary>
		/// <para>ドローコール</para>
		/// <para>直前のドローコールで自身の RT にレンダリングされ、</para>
		/// <para>今回のドローコールでは与えられた RT に対してレンダリングする</para>
		/// </summary>
		void Draw(
			const RenderContext& renderContext,
			const RenderTargetContext& renderTargetContext
		) const
		{
			D3D12Utils::Draw(
				renderContext.commandList, renderContext.commandQueue, renderContext.commandAllocator, renderContext.device,
				rootSignature, pipelineState, renderTargetContext.rt,
				renderTargetContext.rtv, dsv_Dummy, descriptorHeapBasic, renderMeshContext.vbvList, renderMeshContext.ibvList,
				GraphicsBufferState::Present, GraphicsBufferState::RenderTarget,
				renderTargetContext.viewportScissorRect, PrimitiveTopology::TriangleList,
				true, Color::Transparent(), DepthBufferClearValue,
				renderMeshContext.indexCountList
			);
		}

	private:
		RootSignature rootSignature;
		PipelineState pipelineState;
		GraphicsBuffer rt;
		DescriptorHandleAtCPU rtv;
		DescriptorHandleAtCPU dsv_Dummy;
		DescriptorHeap descriptorHeapBasic;
		RenderMeshContext renderMeshContext;

		int cbCount;
		int srCount; // 実際は RT,SR 用のバッファも追加で1つある
	};
}
