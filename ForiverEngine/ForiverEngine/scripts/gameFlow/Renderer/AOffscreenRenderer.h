#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

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

		// <para>初期化. 描画に必要なオブジェクトを作成する</para>
		// <para>使い方の例としては、基底クラスのコンストラクタで CB, SR 群などを作成し、そのコンストラクタの最後でこの Init() を呼び出すとか</para>
		void Init(
			const Device& device, const Lattice2& windowSize,
			const std::vector<GraphicsBuffer>& cbs,                     // 順に b0~ にバインドされる
			const std::vector<std::pair<GraphicsBuffer, Texture>>& srs, // 順に t1~ にバインドされる (t0 は RT/SR 用)
			const std::string& shaderFilePath
		)
		{
			cbCount = static_cast<int>(cbs.size());
			srCount = static_cast<int>(srs.size());

			// RT,SR となるバッファを作成
			const Texture rtMetadata = Texture::CreateManually({}, windowSize, Format::RGBA_U8_01);
			rt = D3D12Helper::CreateGraphicsBufferTexture2D(device, rtMetadata,
				GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color::Transparent());

			// RootSignature, PipelineState
			const RootParameter rootParameter = RootParameter::CreateBasic(cbCount, srCount + 1);
			const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
			const auto [shaderVS, shaderPS] = D3D12Utils::CompileShader_VS_PS(shaderFilePath);
			std::tie(rootSignature, pipelineState) = D3D12Utils::CreateRootSignatureAndGraphicsPipelineState(
				device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, false);

			// RTVのみ作成
			rtv = D3D12Utils::InitRTV(device, rt, Format::RGBA_U8_01);
			dsv_Dummy = DescriptorHandleAtCPU{ .ptr = NULL };

			// メッシュ
			mesh = MeshQuad::CreateFullSized();
			// VBV, IBV
			std::tie(vbv, ibv) = D3D12Utils::CreateMeshViews(device, mesh);

			// DescriptorHeap
			descriptorHeapBasic = D3D12Utils::InitDescriptorHeapBasic(device, cbs, srs);
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
		void ReUploadTexture(const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Texture& texture, ShaderRegister shaderRegister
		)
		{
			const GraphicsBuffer sr = D3D12Utils::InitSR(device, commandList, commandQueue, commandAllocator, texture);
			D3D12Helper::CreateSRVAndRegistToDescriptorHeap(device, descriptorHeapBasic, sr,
				static_cast<int>(shaderRegister) + cbCount, texture);
		}

		/// <summary>
		/// <para>ドローコール</para>
		/// <para>直前のドローコールで自身の RT にレンダリングされ、</para>
		/// <para>今回のドローコールでは次の RT に対してレンダリングする</para>
		/// </summary>
		void Draw(
			const CommandList& commandList,
			const CommandQueue& commandQueue,
			const CommandAllocator& commandAllocator,
			const Device& device,
			const GraphicsBuffer& rt,
			const DescriptorHandleAtCPU& rtv,
			const ViewportScissorRect& viewportScissorRect
		) const
		{
			D3D12Utils::Draw(
				commandList, commandQueue, commandAllocator, device,
				rootSignature, pipelineState, rt,
				rtv, dsv_Dummy, descriptorHeapBasic, { vbv }, { ibv },
				GraphicsBufferState::Present, GraphicsBufferState::RenderTarget,
				viewportScissorRect, PrimitiveTopology::TriangleList, Color::Transparent(), DepthBufferClearValue,
				{ static_cast<int>(mesh.indices.size()) }
			);
		}

	private:
		RootSignature rootSignature;
		PipelineState pipelineState;
		GraphicsBuffer rt;
		DescriptorHandleAtCPU rtv;
		DescriptorHandleAtCPU dsv_Dummy;
		DescriptorHeap descriptorHeapBasic;
		MeshQuad mesh;
		VertexBufferView vbv;
		IndexBufferView ibv;

		int cbCount;
		int srCount; // 実際は RT,SR 用のバッファも追加で1つある
	};
}
