#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "./AImageRenderer.h"

namespace ForiverEngine
{
	/// <summary>
	/// ポインタ(画面中央にあるクロスヘア)のレンダラー
	/// </summary>
	class PointerImageRenderer : public AImageRenderer
	{
	private:
		using Base = AImageRenderer;

	public:

		explicit PointerImageRenderer(
			const Device& device,
			const CommandList& commandList, const CommandQueue& commandQueue, const CommandAllocator& commandAllocator,
			const Lattice2& windowSize
		)
		{
			Base::Init(
				device, commandList, commandQueue, commandAllocator, windowSize,
				"assets/textures/pointer.png",
				windowSize / 2,
				Vector2::One() * 3.0f
			);
		}
	};
}
