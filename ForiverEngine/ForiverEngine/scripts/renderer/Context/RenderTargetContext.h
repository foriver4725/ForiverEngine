#pragma once

#include "scripts/renderer/IncludeInternal.h"

namespace ForiverEngine
{
	/// <summary>
	/// どこにレンダリングするか
	/// </summary>
	struct RenderTargetContext
	{
		GraphicsBuffer rt;
		DescriptorHandleAtCPU rtv;
		ViewportScissorRect viewportScissorRect;
	};
}
