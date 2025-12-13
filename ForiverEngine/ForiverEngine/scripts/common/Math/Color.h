#pragma once

#include "./Defines.h"
#include <stdint.h>
#include <algorithm>

namespace ForiverEngine
{
	struct Color
	{
		float r;
		float g;
		float b;
		float a;

		constexpr Color() noexcept : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
		constexpr Color(float r, float g, float b, float a = 1.0f) noexcept : r(r), g(g), b(b), a(a) {}
		constexpr Color(const Color& other) noexcept : r(other.r), g(other.g), b(other.b), a(other.a) {}
		constexpr Color(Color&& other) noexcept : r(other.r), g(other.g), b(other.b), a(other.a) {}

		constexpr Color(const Vector4& vec) noexcept : r(vec.x), g(vec.y), b(vec.z), a(vec.w) {}
		constexpr Color(const Vector3& vec, float a = 1.0f) noexcept : r(vec.x), g(vec.y), b(vec.z), a(a) {}
		constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 0xff) noexcept :
			r(1.0f * r / 0xff),
			g(1.0f * g / 0xff),
			b(1.0f * b / 0xff),
			a(1.0f * a / 0xff)
		{
		}

		static constexpr Color Transparent() noexcept { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
		static constexpr Color Black() noexcept { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
		static constexpr Color White() noexcept { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
		static constexpr Color Red() noexcept { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
		static constexpr Color Green() noexcept { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
		static constexpr Color Blue() noexcept { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
		static constexpr Color Yellow() noexcept { return Color(1.0f, 1.0f, 0.0f, 1.0f); }
		static constexpr Color Magenta() noexcept { return Color(1.0f, 0.0f, 1.0f, 1.0f); }
		static constexpr Color Cyan() noexcept { return Color(0.0f, 1.0f, 1.0f, 1.0f); }

		Color& operator=(const Color& other) noexcept
		{
			if (this != &other)
			{
				r = other.r;
				g = other.g;
				b = other.b;
				a = other.a;
			}
			return *this;
		}
		Color& operator=(Color&& other) noexcept
		{
			if (this != &other)
			{
				r = other.r;
				g = other.g;
				b = other.b;
				a = other.a;
			}
			return *this;
		}

		bool operator==(const Color& other) const noexcept
		{
			return std::abs(r - other.r) < Epsilon
				&& std::abs(g - other.g) < Epsilon
				&& std::abs(b - other.b) < Epsilon
				&& std::abs(a - other.a) < Epsilon;
		}
		bool operator!=(const Color& other) const noexcept
		{
			return !(*this == other);
		}

		Color operator+(const Color& other) const noexcept
		{
			return Color(r + other.r, g + other.g, b + other.b, a + other.a);
		}
		Color operator-(const Color& other) const noexcept
		{
			return Color(r - other.r, g - other.g, b - other.b, a - other.a);
		}
		Color operator*(const Color& other) const noexcept
		{
			return Color(r * other.r, g * other.g, b * other.b, a * other.a);
		}
		Color operator/(const Color& other) const noexcept
		{
			return Color(
				(std::abs(other.r) < Epsilon) ? 0.0f : (r / other.r),
				(std::abs(other.g) < Epsilon) ? 0.0f : (g / other.g),
				(std::abs(other.b) < Epsilon) ? 0.0f : (b / other.b),
				(std::abs(other.a) < Epsilon) ? 0.0f : (a / other.a)
			);
		}
		Color operator*(float scalar) const noexcept
		{
			return Color(r * scalar, g * scalar, b * scalar, a * scalar);
		}
		Color operator/(float scalar) const noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				return Color::White();
			}
			return Color(r / scalar, g / scalar, b / scalar, a / scalar);
		}
		friend Color operator*(float scalar, const Color& color) noexcept
		{
			return Color(color.r * scalar, color.g * scalar, color.b * scalar, color.a * scalar);
		}

		void operator+=(const Color& other) noexcept
		{
			r += other.r;
			g += other.g;
			b += other.b;
			a += other.a;
		}
		void operator-=(const Color& other) noexcept
		{
			r -= other.r;
			g -= other.g;
			b -= other.b;
			a -= other.a;
		}
		void operator*=(const Color& other) noexcept
		{
			r *= other.r;
			g *= other.g;
			b *= other.b;
			a *= other.a;
		}
		void operator/=(const Color& other) noexcept
		{
			r = (std::abs(other.r) < Epsilon) ? 0.0f : (r / other.r);
			g = (std::abs(other.g) < Epsilon) ? 0.0f : (g / other.g);
			b = (std::abs(other.b) < Epsilon) ? 0.0f : (b / other.b);
			a = (std::abs(other.a) < Epsilon) ? 0.0f : (a / other.a);
		}
		void operator*=(float scalar) noexcept
		{
			r *= scalar;
			g *= scalar;
			b *= scalar;
			a *= scalar;
		}
		void operator/=(float scalar) noexcept
		{
			if (std::abs(scalar) < Epsilon)
			{
				r = 0.0f;
				g = 0.0f;
				b = 0.0f;
				a = 0.0f;
			}
			else
			{
				r /= scalar;
				g /= scalar;
				b /= scalar;
				a /= scalar;
			}
		}

		static Color Lerp(const Color& from, const Color& to, float t) noexcept
		{
			const float _t = std::clamp(t, 0.0f, 1.0f);
			return Color(
				from.r + (to.r - from.r) * _t,
				from.g + (to.g - from.g) * _t,
				from.b + (to.b - from.b) * _t,
				from.a + (to.a - from.a) * _t
			);
		}

		static Color RGBToHSV(const Color& rgb) noexcept
		{
			float r = rgb.r;
			float g = rgb.g;
			float b = rgb.b;
			float max = std::max({ r, g, b });
			float min = std::min({ r, g, b });
			float delta = max - min;
			float h = 0.0f;
			if (delta > Epsilon)
			{
				if (max == r)
				{
					h = 60.0f * (fmod(((g - b) / delta), 6.0f));
				}
				else if (max == g)
				{
					h = 60.0f * (((b - r) / delta) + 2.0f);
				}
				else // max == b
				{
					h = 60.0f * (((r - g) / delta) + 4.0f);
				}
			}
			if (h < 0.0f)
			{
				h += 360.0f;
			}
			float s = (max <= Epsilon) ? 0.0f : (delta / max);
			float v = max;
			return Color(h / 360.0f, s, v, rgb.a); // h を [0,1] に正規化して返す
		}
		static Color HSVToRGB(const Color& hsv) noexcept
		{
			float h = hsv.r * 360.0f; // [0,360)
			float s = hsv.g;
			float v = hsv.b;

			float c = v * s;
			float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
			float m = v - c;

			float r1 = 0.0f, g1 = 0.0f, b1 = 0.0f;

			if (h >= 0.0f && h < 60.0f)
			{
				r1 = c; g1 = x; b1 = 0.0f;
			}
			else if (h < 120.0f)
			{
				r1 = x; g1 = c; b1 = 0.0f;
			}
			else if (h < 180.0f)
			{
				r1 = 0.0f; g1 = c; b1 = x;
			}
			else if (h < 240.0f)
			{
				r1 = 0.0f; g1 = x; b1 = c;
			}
			else if (h < 300.0f)
			{
				r1 = x; g1 = 0.0f; b1 = c;
			}
			else // h < 360
			{
				r1 = c; g1 = 0.0f; b1 = x;
			}

			return Color(r1 + m, g1 + m, b1 + m, hsv.a);
		}
	};
}
