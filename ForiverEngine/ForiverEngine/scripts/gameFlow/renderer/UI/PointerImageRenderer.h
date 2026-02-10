#pragma once

#include "scripts/gameFlow/IncludeInternal.h"

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
