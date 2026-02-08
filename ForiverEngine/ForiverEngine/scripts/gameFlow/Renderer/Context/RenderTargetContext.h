#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

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
