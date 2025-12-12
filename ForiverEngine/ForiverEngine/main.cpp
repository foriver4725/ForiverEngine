//#define ENABLE_CUI_CONSOLE

#include "scripts/headers/D3D12BasicFlow.h"

constexpr int WindowWidth = 960;
constexpr int WindowHeight = 540;

BEGIN_INITIALIZE(L"DX12Sample", L"DX12 テスト", hwnd, WindowWidth, WindowHeight);
{
	using namespace ForiverEngine;

#ifdef _DEBUG
	if (!D3D12Helper::EnableDebugLayer())
		ShowError(L"DebugLayer の有効化に失敗しました");
#endif

	const auto [factory, device, commandAllocater, commandList, commandQueue, swapChain, descriptorHeapRTV]
		= D3D12BasicFlow::CreateStandardObjects(hwnd, WindowWidth, WindowHeight, false);

	// ルートパラメータ
	const RootParameter rootParameter =
	{
		.shaderVisibility = ShaderVisibility::PixelOnly,
		.descriptorRanges =
		{
			// SRV x1 t0
			{
				.type = RootParameter::DescriptorRangeType::SRV,
				.amount = 1,
				.registerIndex = 0,
			},
		}
	};

	const SamplerConfig samplerConfig =
	{
		.shaderVisibility = ShaderVisibility::PixelOnly,
		.addressingMode = SamplerConfig::AddressingMode::Wrap,
		.filter = SamplerConfig::Filter::Bilinear,
	};

	// 頂点データ
	const std::vector<VertexData> vertices =
	{
		{{ -0.4f, -0.7f, 0, 1 }, {0.0f, 1.0f}},
		{{ -0.4f, 0.7f, 0, 1 },  {0.0f, 0.0f}},
		{{ 0.4f, -0.7f, 0, 1 },  {1.0f, 1.0f}},
		{{ 0.4f, 0.7f, 0, 1 },   {1.0f, 0.0f}},
	};

	// 頂点インデックス
	const std::vector<std::uint16_t> indices =
	{
		0, 1, 2,
		2, 1, 3,
	};

	// 頂点レイアウト
	const std::vector<VertexLayout> vertexLayouts =
	{
		{ "POSITION", Format::RGBA_F32 },
		{ "TEXCOORD", Format::RG_F32 },
	};

	const Texture texture = AssetLoader::LoadTexture("assets/pickaxe.png");
	if (!texture.IsValid())
		ShowError(L"テクスチャのロードに失敗しました");
	const GraphicsBuffer textureCopyIntermediateBuffer = D3D12Helper::CreateGraphicsBuffer1D(device,
		static_cast<int>(GetAlignmentedSize(texture.rowSize, Texture::RowSizeAlignment) * texture.height), true);
	if (!textureCopyIntermediateBuffer)
		ShowError(L"テクスチャ転送用中間バッファの作成に失敗しました");
	const GraphicsBuffer textureBuffer = D3D12Helper::CreateGraphicsBufferTexture2D(device, texture);
	if (!textureBuffer)
		ShowError(L"テクスチャバッファの作成に失敗しました");

	if (!D3D12Helper::CommandCopyDataFromCPUToGPUThroughGraphicsBufferTexture2D(commandList, textureCopyIntermediateBuffer, textureBuffer, texture))
		ShowError(L"テクスチャデータのアップロードに失敗しました");
	D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, textureBuffer,
		GraphicsBufferState::CopyDestination, GraphicsBufferState::PixelShaderResource, false);
	D3D12Helper::CommandClose(commandList);
	D3D12Helper::ExecuteCommands(commandQueue, commandList);
	if (!D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue))
		ShowError(L"GPU の処理待ち受けに失敗しました");

	const DescriptorHeap descriptorHeapSRV = D3D12Helper::CreateDescriptorHeapSRV(device, 1);
	if (!descriptorHeapSRV)
		ShowError(L"SRV 用 DescriptorHeap の作成に失敗しました");
	D3D12Helper::CreateShaderResourceViewAndRegistToDescriptorHeap(textureBuffer, texture.format, device, descriptorHeapSRV);

	const auto [vertexBufferView, indexBufferView]
		= D3D12BasicFlow::CreateVertexAndIndexBufferViews(device, vertices, indices);

	const auto [shaderVS, shaderPS]
		= D3D12BasicFlow::CompileShader_VS_PS("./shaders/Basic.hlsl");

	const ViewportScissorRect viewportScissorRect
		= ViewportScissorRect::CreateFullSized(WindowWidth, WindowHeight);

	const auto [rootSignature, graphicsPipelineState]
		= D3D12BasicFlow::CreateRootSignatureAndGraphicsPipelineState(
			device, rootParameter, samplerConfig, shaderVS, shaderPS, vertexLayouts, FillMode::Solid, CullMode::None);

	BEGIN_MESSAGE_LOOP;
	{
		const auto [currentBackBuffer, currentBackBufferRTV]
			= D3D12BasicFlow::GetCurrentBackBufferAndCreateView(device, swapChain, descriptorHeapRTV);

		D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, currentBackBuffer,
			GraphicsBufferState::Present, GraphicsBufferState::RenderTarget, false);
		{
			D3D12Helper::CommandSetRTAsOutputStage(commandList, currentBackBufferRTV);
			D3D12Helper::CommandClearRT(commandList, currentBackBufferRTV, { 0, 0, 0, 1 });
			D3D12Helper::CommandSetRootSignature(commandList, rootSignature);
			D3D12Helper::CommandSetGraphicsPipelineState(commandList, graphicsPipelineState);
			D3D12Helper::CommandSetDescriptorHeaps(commandList, { descriptorHeapSRV });
			D3D12Helper::CommandLinkRootParameterIndexAndDescriptorHeapHandleAtGPU(
				commandList, device, descriptorHeapSRV, DescriptorHeapType::CBV_SRV_UAV, 0, 0); // ルートパラメーターは1つだけ
			D3D12Helper::CommandIASetPrimitiveTopology(commandList, PrimitiveTopology::TriangleList);
			D3D12Helper::CommandIASetVertexBuffer(commandList, { vertexBufferView });
			D3D12Helper::CommandIASetIndexBuffer(commandList, indexBufferView);
			D3D12Helper::CommandRSSetViewportAndScissorRect(commandList, viewportScissorRect);
			D3D12Helper::CommandDrawIndexedInstanced(commandList, static_cast<int>(indices.size()));
		}
		D3D12Helper::CommandInvokeResourceBarrierAsTransition(commandList, currentBackBuffer,
			GraphicsBufferState::RenderTarget, GraphicsBufferState::Present, false);
		D3D12Helper::CommandClose(commandList);
		D3D12Helper::ExecuteCommands(commandQueue, commandList);

		if (!D3D12Helper::WaitForGPUEventCompletion(D3D12Helper::CreateFence(device), commandQueue))
			ShowError(L"GPU の処理待ち受けに失敗しました");

		if (!D3D12Helper::ClearCommandAllocatorAndList(commandAllocater, commandList))
			ShowError(L"CommandAllocator, CommandList のクリアに失敗しました");

		if (!D3D12Helper::Present(swapChain))
			ShowError(L"画面のフリップに失敗しました");
	}
	END_MESSAGE_LOOP;
}
END_INITIALIZE(0);
