#pragma once

namespace ForiverEngine
{
	struct Vector3;
	struct Vector4;

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

		explicit Color(const Vector3& vec, float a = 1.0f) noexcept;
		explicit Color(Vector3&& vec, float a = 1.0f) noexcept;

		explicit Color(const Vector4& vec) noexcept;
		explicit Color(Vector4&& vec) noexcept;

		static constexpr Color Transparent() noexcept { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
		static constexpr Color Black() noexcept { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
		static constexpr Color White() noexcept { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
		static constexpr Color Red() noexcept { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
		static constexpr Color Green() noexcept { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
		static constexpr Color Blue() noexcept { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
		static constexpr Color Yellow() noexcept { return Color(1.0f, 1.0f, 0.0f, 1.0f); }
		static constexpr Color Magenta() noexcept { return Color(1.0f, 0.0f, 1.0f, 1.0f); }
		static constexpr Color Cyan() noexcept { return Color(0.0f, 1.0f, 1.0f, 1.0f); }

		static constexpr Color CreateFromUint8(int r, int g, int b, int a = 0xff) noexcept
		{
			return Color(
				1.0f * r / 0xff,
				1.0f * g / 0xff,
				1.0f * b / 0xff,
				1.0f * a / 0xff
			);
		}

		Color& operator=(const Color& other) noexcept;
		Color& operator=(Color&& other) noexcept;

		bool operator==(const Color& other) const noexcept;
		bool operator!=(const Color& other) const noexcept;

		Color operator+(const Color& other) const noexcept;
		Color operator-(const Color& other) const noexcept;
		Color operator*(float scalar) const noexcept;
		Color operator/(float scalar) const noexcept;
		friend Color operator*(float scalar, const Color& color) noexcept;

		Color operator*(const Color& other) const noexcept;
		Color operator/(const Color& other) const noexcept;

		Color& operator+=(const Color& other) noexcept;
		Color& operator-=(const Color& other) noexcept;
		Color& operator*=(float scalar) noexcept;
		Color& operator/=(float scalar) noexcept;

		Color& operator*=(const Color& other) noexcept;
		Color& operator/=(const Color& other) noexcept;

		static Color Lerp(const Color& from, const Color& to, float t) noexcept;

		static Color RGBToHSV(const Color& rgb) noexcept;
		static Color HSVToRGB(const Color& hsv) noexcept;
	};
}
