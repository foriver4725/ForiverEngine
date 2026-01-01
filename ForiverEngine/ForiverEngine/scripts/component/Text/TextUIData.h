#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>

namespace ForiverEngine
{
	/// <summary>
	/// 画面に描画するテキストのデータ
	/// </summary>
	class TextUIData
	{
		// 文字の位置は、フォントサイズの倍数ピクセルで固定してしまう
		// 各画面上の矩形(これはインデックスでアクセスする)毎に、その位置に何番の文字を描画するかを指定する
		// インデックスは [0, 255]. 描画しないなら 256 を指定する

	public:
		static constexpr int FontTextureTextLength = 16; // フォントテクスチャの、1文字の幅かつ高さ (px)

		// データのサイズを指定して、空で埋めて作成する
		static TextUIData CreateEmpty(const Lattice2& dataSize)
		{
			TextUIData textUIData;

			textUIData.data = std::vector<std::vector<Text::Data>>(
				dataSize.y,
				std::vector<Text::Data>(
					dataSize.x,
					Text::Data::CreateDefault()
				)
			);

			textUIData.dataSize = dataSize;

			return textUIData;
		}

		Lattice2 GetDataSize() const { return dataSize; }

		// テキストセット 単文字
		void SetText(const Lattice2& positionIndex, char text, const Color& color = Text::DefaultColor)
		{
			data[positionIndex.y][positionIndex.x] = Text::Data
			{
				.color = color,
				.fontTextureIndex = Text::ConvertToFontTextureIndex(text),
			};
		}

		// テキストセット 複数文字
		// 勝手に改行をする. 文字が画面からはみ出るならその分だけ無視する.
		void SetTexts(const Lattice2& beginPositionIndex, const std::string& texts, const Color& color = Text::DefaultColor)
		{
			const int textCount = static_cast<int>(texts.size());
			const int beginDataIndex = beginPositionIndex.y * dataSize.x + beginPositionIndex.x; // 最小値
			const int endDataIndex = std::min(beginDataIndex + textCount, dataSize.x * dataSize.y); // 最大値+1

			for (int i = beginDataIndex; i < endDataIndex; ++i)
			{
				const int xi = i % dataSize.x;
				const int yi = i / dataSize.x;
				const int ti = i - beginDataIndex; // 何番目の文字か

				SetText(Lattice2(xi, yi), texts[ti], color);
			}
		}

		// 行の文字をまとめてクリアする
		void ClearRow(int rowIndex)
		{
			for (int x = 0; x < dataSize.x; ++x)
			{
				data[rowIndex][x] = Text::Data::CreateDefault();
			}
		}

		// 全文字をまとめてクリアする
		// TODO: ちょっと処理が重いか?
		void ClearAll()
		{
			for (int y = 0; y < dataSize.y; ++y)
			{
				ClearRow(y);
			}
		}

		// データからテクスチャを作成する
		// テクスチャ自体は2Dだけど、生データは1Dのビット配列なので注意
		Texture CreateTexture() const
		{
			const int dataSizeTotal = dataSize.x * dataSize.y;

			std::vector<std::uint8_t> textureData;
			textureData.reserve(dataSizeTotal * (sizeof(std::uint8_t) * 4));

			for (int i = 0; i < dataSizeTotal; ++i)
			{
				const int xi = i % dataSize.x;
				const int yi = i / dataSize.x;
				const Text::Data& singleData = data[yi][xi];

				// uint8 にして詰める
				// [0, 255]
				textureData.push_back(static_cast<std::uint8_t>(singleData.color.r * 0xff));
				textureData.push_back(static_cast<std::uint8_t>(singleData.color.g * 0xff));
				textureData.push_back(static_cast<std::uint8_t>(singleData.color.b * 0xff));
				textureData.push_back(static_cast<std::uint8_t>(singleData.fontTextureIndex));
			}

			return TextureLoader::CreateManually(textureData, dataSize.x, dataSize.y, Format::RGBA_U8);
		}

	private:
		// y, x の順番
		std::vector<std::vector<Text::Data>> data;

		// 毎回計算するのは面倒なので、作成時にX,Yの要素数をキャッシュしておく
		// ファクトリメソッド内で、忘れずにセットすること!
		Lattice2 dataSize;
	};
}
