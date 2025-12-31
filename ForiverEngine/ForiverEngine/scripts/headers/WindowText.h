#pragma once

#include <scripts/common/Include.h>
#include "./TextureLoader.h"

namespace ForiverEngine
{
	/// <summary>
	/// 画面に描画するテキストのデータ
	/// </summary>
	class WindowText
	{
		// 文字の位置は、フォントサイズの倍数ピクセルで固定してしまう
		// 各画面上の矩形(これはインデックスでアクセスする)毎に、その位置に何番の文字を描画するかを指定する
		// インデックスは [0, FontTextureIndexInt.max - 1]. 描画しないなら FontTextureIndexInt.max を指定する

	public:
		using FontTextureIndexInt = std::uint8_t; // フォントテクスチャでのインデックス (8bit 以外にする場合、テクスチャへのデータパッキングを再考すること!)
		static constexpr FontTextureIndexInt NoTextFontTextureIndex = std::numeric_limits<FontTextureIndexInt>::max();
		static constexpr int FontSingleLength = 16; // 1フォント文字の幅/高さ (px)

		struct SingleData
		{
			Color color; // a は使わず、インデックスデータを詰めて 16bit にする
			FontTextureIndexInt fontIndex;
		};

		// ファクトリメソッド

		// 文字が何個入るかだけ指定して、実際の文字は入れずに作成する
		static WindowText CreateEmpty(const Lattice2& fontCount)
		{
			const int maxTextCount = GetMaxTextCount(fontCount);

			WindowText windowText;

			windowText.data = std::vector<std::vector<SingleData>>(
				fontCount.y,
				std::vector<SingleData>(
					fontCount.x,
					CreateDefaultSingleData()
				)
			);

			windowText.count = fontCount;

			return windowText;
		}

		// アクセサ (インデックスの境界チェックはしない!)

		// 配列のサイズ
		Lattice2 GetCount() const
		{
			return count;
		}

		// 単文字
		void SetText(const Lattice2& positionIndex, char text, const Color& color = Color::Black())
		{
			data[positionIndex.y][positionIndex.x] = SingleData
			{
				.color = color,
				.fontIndex = GetFontIndex(text),
			};
		}

		// 複数文字
		// 勝手に改行をする. 文字が画面からはみ出るならその分だけ無視する.
		void SetTexts(const Lattice2& beginPositionIndex, const std::string& texts, const Color& color = Color::Black())
		{
			const int textCount = static_cast<int>(texts.size());
			const int beginDataIndex = beginPositionIndex.y * count.x + beginPositionIndex.x; // 最小値
			const int endDataIndex = std::min(beginDataIndex + textCount, static_cast<int>(GetMaxTextCount())); // 最大値+1

			for (int i = beginDataIndex; i < endDataIndex; ++i)
			{
				const int xi = i % count.x;
				const int yi = i / count.x;
				const int ti = i - beginDataIndex; // 何番目の文字か

				SetText(Lattice2(xi, yi), texts[ti], color);
			}
		}

		// 行の文字をまとめてクリアする
		void ClearRow(int rowIndex)
		{
			for (int x = 0; x < count.x; ++x)
			{
				data[rowIndex][x] = CreateDefaultSingleData();
			}
		}

		// 全文字をまとめてクリアする
		// TODO: ちょっと処理が重いか?
		void ClearAll()
		{
			for (int y = 0; y < count.y; ++y)
			{
				ClearRow(y);
			}
		}

		// データからテクスチャに変換する
		// テクスチャ自体は2Dだけど、生データは1Dのビット配列なので注意
		Texture CreateTexture() const
		{
			// データサイズとかは、直書きしてしまう

			const int pixelCount = GetMaxTextCount();
			const int texelValueSize = 16;

			std::vector<std::uint8_t> data;
			data.reserve(pixelCount * texelValueSize);

			for (int i = 0; i < pixelCount; ++i)
			{
				const int xi = i % count.x;
				const int yi = i / count.x;
				const SingleData& singleData = this->data[yi][xi];

				// uint8 にして詰める
				// [0, 255]
				data.push_back(static_cast<std::uint8_t>(singleData.color.r * 0xff));
				data.push_back(static_cast<std::uint8_t>(singleData.color.g * 0xff));
				data.push_back(static_cast<std::uint8_t>(singleData.color.b * 0xff));
				data.push_back(static_cast<std::uint8_t>(singleData.fontIndex));
			}

			return TextureLoader::CreateManually(data, count.x, count.y, Format::RGBA_U8);
		}

		constexpr int GetMaxTextCount() const
		{
			return count.x * count.y;
		}
		static constexpr int GetMaxTextCount(const Lattice2& xyCount)
		{
			return xyCount.x * xyCount.y;
		}

		static constexpr SingleData CreateDefaultSingleData()
		{
			return SingleData
			{
				.color = Color::Black(),
				.fontIndex = NoTextFontTextureIndex,
			};
		}

		static constexpr FontTextureIndexInt GetFontIndex(char text)
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
			default: return NoTextFontTextureIndex; // 無効な文字が来たら、描画しないようにする
			}
		}

	private:
		// y, x の順番
		// 実際のテクスチャにおけるインデックスが入る
		std::vector<std::vector<SingleData>> data;

		// 毎回計算するのは面倒なので、作成時にX,Yの要素数をキャッシュしておく
		// ファクトリメソッド内で、忘れずにセットすること!
		Lattice2 count;
	};
}
