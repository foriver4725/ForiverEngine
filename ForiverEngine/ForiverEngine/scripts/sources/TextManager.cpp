#include "../headers/TextManager.h"

#if _DEBUG
#pragma comment(lib, "DirectXTK12_x64_Debug.lib")
#else
#pragma comment(lib, "DirectXTK12_x64_Release.lib")
#endif

namespace ForiverEngine
{
	void TextManager::Init(const Device& device, const std::string& fontPath,
		const DescriptorHeapHandleAtCPU& descriptorAtCPU, const DescriptorHeapHandleAtGPU& descriptorAtGPU)
	{
		if (TextManager::hasInitialized) return;
		TextManager::hasInitialized = true;

		TextManager::graphicsMemory = new DirectX::GraphicsMemory(device.Ptr);

		DirectX::ResourceUploadBatch resourceUploadBatch(device.Ptr);
		resourceUploadBatch.Begin();

		const DirectX::RenderTargetState renderTargetState(
			static_cast<DXGI_FORMAT>(Format::RGBA_U8_01), // RT のフォーマット
			static_cast<DXGI_FORMAT>(Format::D_F32) // DS のフォーマット
		);
		const DirectX::SpriteBatchPipelineStateDescription spriteBatchPipelineStateDesc(renderTargetState);
		TextManager::spriteBatch = new DirectX::SpriteBatch(device.Ptr, resourceUploadBatch, spriteBatchPipelineStateDesc);

		const D3D12_CPU_DESCRIPTOR_HANDLE* descriptorAtCPURealPtr
			= reinterpret_cast<const D3D12_CPU_DESCRIPTOR_HANDLE*>(const_cast<DescriptorHeapHandleAtCPU*>(&descriptorAtCPU));
		const D3D12_GPU_DESCRIPTOR_HANDLE* descriptorAtGPURealPtr
			= reinterpret_cast<const D3D12_GPU_DESCRIPTOR_HANDLE*>(const_cast<DescriptorHeapHandleAtGPU*>(&descriptorAtGPU));
		TextManager::spriteFont = new DirectX::SpriteFont(
			device.Ptr,
			resourceUploadBatch,
			StringUtils::UTF8ToUTF16(fontPath).c_str(),
			*descriptorAtCPURealPtr,
			*descriptorAtGPURealPtr
		);
	}
}
