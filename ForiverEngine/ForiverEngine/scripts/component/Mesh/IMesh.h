#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>

namespace ForiverEngine
{
	template<typename TVertexData>
	struct IMesh
	{
		virtual ~IMesh() = default;

		virtual const std::vector<TVertexData>& GetVertices() const = 0;
		virtual const std::vector<std::uint32_t>& GetIndices() const = 0;
	};
}
