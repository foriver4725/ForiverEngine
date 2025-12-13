#pragma once

#include <cmath>

namespace ForiverEngine
{
	// 浮動小数点数の誤差許容値
	constexpr float Epsilon = 1.0e-6f;

	// 円周率
	constexpr float Pi = 3.14159265358979323846f;
	constexpr float DoublePi = Pi * 2.0f;
	constexpr float HalfPi = Pi * 0.5f;

	// 角度変換定数 (ラジアン <-> 度)
	constexpr float RadToDeg = 180.0f / Pi;
	constexpr float DegToRad = Pi / 180.0f;

	// ネイピア数
	constexpr float E = 2.71828182845904523536f;

	// 重力加速度 (m/s²)
	constexpr float G = 9.80665f;
}
