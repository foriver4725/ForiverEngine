#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// レンダリングで使うもの
	/// </summary>
	struct RenderContext
	{
		Device device;
		CommandList commandList;
		CommandQueue commandQueue;
		CommandAllocator commandAllocator;
	};
}
