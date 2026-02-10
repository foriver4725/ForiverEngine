#pragma once

#include "scripts/renderer/IncludeInternal.h"

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
