#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// 文字1つに関するヘルパークラス
	/// </summary>
	class Text final
	{
	public:
		DELETE_DEFAULT_METHODS(Text);

		static constexpr Color DefaultColor = Color::Black();
		// 無効文字. この値なら描画しないことを表す
		static constexpr std::uint8_t InvalidFontTextureIndex = 0xff;

		struct Data
		{
			Color color; // a は使わない. 代わりにインデックスを詰めて GPU にアップロードする想定
			std::uint8_t fontTextureIndex;

			static constexpr Data CreateDefault()
			{
				return Data
				{
					.color = DefaultColor,
					.fontTextureIndex = InvalidFontTextureIndex,
				};
			}
		};

		static constexpr std::uint8_t ConvertToFontTextureIndex(char text)
		{
			switch (text)
			{
				// 大文字と小文字は同一視する

			case 'A': return 0; case 'a': return 0;
			case 'B': return 1; case 'b': return 1;
			case 'C': return 2; case 'c': return 2;
			case 'D': return 3; case 'd': return 3;
			case 'E': return 4; case 'e': return 4;
			case 'F': return 5; case 'f': return 5;
			case 'G': return 6; case 'g': return 6;
			case 'H': return 7; case 'h': return 7;

			case 'I': return 8; case 'i': return 8;
			case 'J': return 9; case 'j': return 9;
			case 'K': return 10; case 'k': return 10;
			case 'L': return 11; case 'l': return 11;
			case 'M': return 12; case 'm': return 12;
			case 'N': return 13; case 'n': return 13;
			case 'O': return 14; case 'o': return 14;
			case 'P': return 15; case 'p': return 15;

			case 'Q': return 16; case 'q': return 16;
			case 'R': return 17; case 'r': return 17;
			case 'S': return 18; case 's': return 18;
			case 'T': return 19; case 't': return 19;
			case 'U': return 20; case 'u': return 20;
			case 'V': return 21; case 'v': return 21;
			case 'W': return 22; case 'w': return 22;
			case 'X': return 23; case 'x': return 23;

			case 'Y': return 24; case 'y': return 24;
			case 'Z': return 25; case 'z': return 25;
			case ':': return 26;
			case '!': return 27;
			case '?': return 28;
			case '.': return 29;
			case ',': return 30;
			case '~': return 31;

			case '0': return 32;
			case '1': return 33;
			case '2': return 34;
			case '3': return 35;
			case '4': return 36;
			case '5': return 37;
			case '6': return 38;
			case '7': return 39;

			case '8': return 40;
			case '9': return 41;
			case '(': return 42;
			case ')': return 43;
			case '[': return 44;
			case ']': return 45;
			case '|': return 46;
			case '_': return 47;

			case '/': return 48;
			case '\\': return 49;
			case '+': return 50;
			case '-': return 51;
			case '#': return 52;
			case '%': return 53;
			case '&': return 54;
			case '$': return 55;

			case '\'': return 56;
			case '"': return 57;
			case '*': return 58;
			case ';': return 59;
			case '=': return 60;
			case '^': return 61;
			case '<': return 62;
			case '>': return 63;

			default: return InvalidFontTextureIndex;
			}
		}
	};
}
