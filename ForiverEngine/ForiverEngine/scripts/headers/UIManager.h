#pragma once

#include <scripts/common/Include.h>

// テキスト用
// TODO: 外部には見せたくない
#include <ResourceUploadBatch.h>
#include <SpriteFont.h>

namespace ForiverEngine
{
	class UIManager
	{
	private:
		// テキスト用
		inline static DirectX::GraphicsMemory* graphicsMemory = nullptr;
		inline static DirectX::SpriteBatch* spriteBatch = nullptr;
		inline static DirectX::SpriteFont* spriteFont = nullptr;

	public:
		static void InitText(const Device& device, const std::string& fontPath,
			const DescriptorHeapHandleAtCPU& descriptorAtCPU, const DescriptorHeapHandleAtGPU& descriptorAtGPU);
	};
}
