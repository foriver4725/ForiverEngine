#pragma once

#include "./Defines.h"
#include "./Vector3.h"
#include "./Quaternion.h"

namespace ForiverEngine
{
	struct Transform
	{
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;
	};
}
