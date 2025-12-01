#include "../headers/Math.h"

#include "../oss/SimplexNoise.h"

namespace ForiverEngine
{
	float Math::SimplexNoise1D(float x)
	{
		return SimplexNoise::noise(x);
	}

	float Math::SimplexNoise2D(float x, float y)
	{
		return SimplexNoise::noise(x, y);
	}

	float Math::SimplexNoise3D(float x, float y, float z)
	{
		return SimplexNoise::noise(x, y, z);
	}
}
