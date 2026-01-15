#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./D3D12BasicFlow.h"

namespace ForiverEngine
{
	class OffscreenRenderer
	{
	public:
		/// <summary>
		/// <para>オフスクリーンレンダラーを作成する</para>
		/// <para>RT, SR 両方に使えるバッファを一つ作成し、それを t0 にバインドする</para>
		/// <para>cbData, srTextures はそれぞれ b0, t1~ にバインドされる</para>
		/// <para>CB は1つのみ設定可能</para>
		/// </summary>
		template<typename TCBData>
			requires (!std::same_as<std::decay_t<TCBData>, OffscreenRenderer>)
		explicit OffscreenRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize,
			const std::string& shaderFilePath,
			const TCBData& cbData, const std::vector<Texture>& srTextures
		)
		{
			cbCount = 1;
			srCount = static_cast<int>(srTextures.size());

			// RT,SR となるバッファを作成
			const Texture rtMetadata = Texture::CreateManually({}, windowSize, Format::RGBA_U8_01);
			rt = D3D12Helper::CreateGraphicsBufferTexture2D(device, rtMetadata,
				GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color::Transparent());

			// RootSignature, PipelineState
			const RootParameter rootParameter = RootParameter::CreateBasic(cbCount, srCount + 1);
			const SamplerConfig samplerConfig = SamplerConfig::CreateBasic(AddressingMode::Clamp, Filter::Point);
			const auto [shaderVS, shaderPS] = D3D12BasicFlow::CompileShader_VS_PS(shaderFilePath);
			std::tie(rootSignature, pipelineState) = D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
				device, rootParameter, samplerConfig, shaderVS, shaderPS, VertexLayoutsQuad, FillMode::Solid, CullMode::Back, false);

			// RTVのみ作成
			rtv = D3D12BasicFlow::InitRTV(device, rt, Format::RGBA_U8_01);
			dsv_Dummy = DescriptorHandleAtCPU{ .ptr = NULL };

			// メッシュ
			mesh = MeshQuad::CreateFullSized();
			// VBV, IBV
			meshViews = D3D12BasicFlow::CreateMeshViews(device, mesh);

			// CB
			const GraphicsBuffer cb = D3D12BasicFlow::InitCBVBuffer(device, cbData);
			const std::vector<GraphicsBuffer> cbs = { cb };

			// SR
			std::vector<std::pair<GraphicsBuffer, Texture>> srs = {};
			srs.reserve(srCount + 1);
			srs.push_back({ rt, rtMetadata }); // RT,SR 用バッファを t0 に設定
			for (const Texture& srTexture : srTextures)
			{
				const GraphicsBuffer sr = D3D12BasicFlow::InitSRVBuffer(
					device, commandList, commandQueue, commandAllocator, srTexture);
				srs.push_back({ sr, srTexture });
			}

			// DescriptorHeap
			descriptorHeapBasic = D3D12BasicFlow::InitDescriptorHeapBasic(device, cbs, srs);
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
			const GraphicsBuffer sr = D3D12BasicFlow::InitSRVBuffer(device, commandList, commandQueue, commandAllocator, texture);
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
			D3D12BasicFlow::Draw(
				commandList, commandQueue, commandAllocator, device,
				rootSignature, pipelineState, rt,
				rtv, dsv_Dummy, descriptorHeapBasic, { meshViews.vbv }, { meshViews.ibv },
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
		MeshViews meshViews;

		int cbCount;
		int srCount; // 実際は RT,SR 用のバッファも追加で1つある
	};
}
