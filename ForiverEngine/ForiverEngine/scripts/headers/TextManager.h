#pragma once

#include <scripts/common/Include.h>

// TODO: ŠO•”‚É‚ÍŒ©‚¹‚½‚­‚È‚¢
#include <ResourceUploadBatch.h>
#include <SpriteFont.h>

namespace ForiverEngine
{
	class TextManager final
	{
	private:
		inline static bool hasInitialized = false;
		inline static DirectX::GraphicsMemory* graphicsMemory = nullptr;
		inline static DirectX::SpriteBatch* spriteBatch = nullptr;
		inline static DirectX::SpriteFont* spriteFont = nullptr;

	public:
		DELETE_DEFAULT_METHODS(TextManager);

		static void Init(const Device& device, const std::string& fontPath,
			const DescriptorHeapHandleAtCPU& descriptorAtCPU, const DescriptorHeapHandleAtGPU& descriptorAtGPU);
	};
}
