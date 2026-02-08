#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include "scripts/gameFlow/Renderer/Context/Include.h"
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

		explicit PointerImageRenderer(const RenderContext& renderContext, const Lattice2& windowSize)
		{
			Base::Init(
				renderContext, windowSize,
				"assets/textures/ui/pointer.png",
				windowSize / 2, Vector2::Zero(), Vector2::One(), Lattice2(24, 24),
				true
			);
		}
	};
}
