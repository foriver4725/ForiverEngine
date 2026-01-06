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
		/// <para>constantBuffers, shaderResourceBuffers はそれぞれ b0~, t1~ にバインドされる</para>
		/// </summary>
		OffscreenRenderer(
			const Device& device,
			const Lattice2& windowSize,
			const std::string& shaderFilePath,
			const std::vector<GraphicsBuffer>& constantBuffers,
			const std::vector<std::pair<GraphicsBuffer, Texture>>& shaderResourceBuffers
		)
		{
			// RT,SR となるバッファを作成
			const Texture rtMetadata = Texture::CreateManually({}, windowSize, Format::RGBA_U8_01);
			rt = D3D12Helper::CreateGraphicsBufferTexture2D(device, rtMetadata,
				GraphicsBufferUsagePermission::AllowRenderTarget, GraphicsBufferState::PixelShaderResource, Color::Transparent());

			// RootSignature, PipelineState
			const RootParameter rootParameter = RootParameter::CreateBasic(
				static_cast<int>(constantBuffers.size()), static_cast<int>(shaderResourceBuffers.size() + 1));
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

			// 作成したバッファ (RT,SR) を先頭に追加する (t0)
			std::vector<std::pair<GraphicsBuffer, Texture>> shaderResourceBuffersReal = { { rt, rtMetadata } };
			shaderResourceBuffersReal.insert(shaderResourceBuffersReal.end(),
				shaderResourceBuffers.begin(), shaderResourceBuffers.end());

			// DescriptorHeap
			descriptorHeapBasic = D3D12BasicFlow::InitDescriptorHeapBasic(device, constantBuffers, shaderResourceBuffersReal);
		}

		const GraphicsBuffer& GetRT() const
		{
			return rt;
		}
		const DescriptorHandleAtCPU& GetRTV() const
		{
			return rtv;
		}
		const DescriptorHeap& GetDescriptorHeapBasic() const
		{
			return descriptorHeapBasic;
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
			D3D12BasicFlow::CommandBasicLoop(
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
	};
}
