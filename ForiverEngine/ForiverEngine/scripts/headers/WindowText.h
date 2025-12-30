#pragma once

#include <scripts/common/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// 画面に描画するテキストのデータ
	/// </summary>
	class WindowText
	{
		// 文字の位置は、フォントサイズの倍数ピクセルで固定してしまう
		// 各画面上の矩形(これはインデックスでアクセスする)毎に、その位置に何番の文字を描画するかを指定する
		// インデックスは [0, GetMaxTextCount()-1]. 描画しないなら GetMaxTextCount() を指定する

	public:
		// テクスチャ内のインデックス や データ内のインデックス など、内部計算は全てこの整数型を用いて行う
		using TextInt = std::uint16_t;
		static constexpr TextInt FontSingleLength = 16; // 1フォント文字の幅/高さ (px)

		// ファクトリメソッド

		// 文字が何個入るかだけ指定して、実際の文字は入れずに作成する
		static WindowText CreateEmpty(const Lattice2& fontCount)
		{
			const TextInt maxTextCount = GetMaxTextCount(fontCount);

			WindowText windowText;

			windowText.data = std::vector<std::vector<TextInt>>(
				fontCount.y,
				std::vector<TextInt>(
					fontCount.x,
					maxTextCount // 初期値は描画しない
				)
			);

			windowText.count = fontCount;

			return windowText;
		}

		// アクセサ (インデックスの境界チェックはしない!)

		// 単文字
		void SetText(const Lattice2& positionIndex, char text)
		{
			data[positionIndex.y][positionIndex.x] = GetFontIndex(text);
		}

		// 複数文字
		// 勝手に改行をする. 文字が画面からはみ出るならその分だけ無視する.
		void SetTexts(const Lattice2& beginPositionIndex, const std::string& texts)
		{
			const TextInt textCount = static_cast<TextInt>(texts.size());
			const TextInt beginDataIndex = static_cast<TextInt>(beginPositionIndex.y * count.x + beginPositionIndex.x); // 最小値
			const TextInt endDataIndex = std::min(static_cast<TextInt>(beginDataIndex + textCount), GetMaxTextCount()); // 最大値+1

			for (TextInt i = beginDataIndex; i < endDataIndex; ++i)
			{
				const TextInt xi = i % count.x;
				const TextInt yi = i / count.x;
				const TextInt ti = i - beginDataIndex; // 何番目の文字か

				SetText(Lattice2(xi, yi), texts[ti]);
			}
		}

		constexpr TextInt GetMaxTextCount() const
		{
			return static_cast<TextInt>(count.x * count.y);
		}
		static constexpr TextInt GetMaxTextCount(const Lattice2& xyCount)
		{
			return static_cast<TextInt>(xyCount.x * xyCount.y);
		}

		constexpr TextInt GetFontIndex(char text) const
		{
			switch (text)
			{
			case 'A': return 0;
			case 'B': return 1;
			case 'C': return 2;
			case 'D': return 3;
			case 'E': return 4;
			case 'F': return 5;
			case 'G': return 6;
			case 'H': return 7;
			case 'I': return 8;
			case 'J': return 9;
			case 'K': return 10;
			case 'L': return 11;
			case 'M': return 12;
			case 'N': return 13;
			case 'O': return 14;
			case 'P': return 15;
			case 'Q': return 16;
			case 'R': return 17;
			case 'S': return 18;
			case 'T': return 19;
			case 'U': return 20;
			case 'V': return 21;
			case 'W': return 22;
			case 'X': return 23;
			case 'Y': return 24;
			case 'Z': return 25;
				// 26番は空き文字になっている. 現状の設計では、わざわざ文字にする必要はない
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
			case '@': return 59;
			case '=': return 60;
			case '^': return 61;
			case '<': return 62;
			case '>': return 63;
			default: return GetMaxTextCount(); // 無効な文字が来たら、描画しないようにする
			}
		}

	private:
		// y, x の順番
		// 実際のテクスチャにおけるインデックスが入る
		std::vector<std::vector<TextInt>> data;

		// 毎回計算するのは面倒なので、作成時にX,Yの要素数をキャッシュしておく
		// ファクトリメソッド内で、忘れずにセットすること!
		Lattice2 count;
	};
}
