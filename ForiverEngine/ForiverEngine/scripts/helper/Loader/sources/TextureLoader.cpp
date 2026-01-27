#include "../headers/TextureLoader.h"

#include <oss/miniz.h>

namespace ForiverEngine
{
	// 4バイトをビッグエンディアンの uint32_t として解釈する
	// ex: {0x00,0x00,0x01,0x0A} → 0x0000010A (266)
	static std::uint32_t ReinterpretAsBigEndianUint32(const std::uint8_t bytes[]);

	// 4バイトが指定された文字列と一致するかチェックする
	static bool CompareType(const std::uint8_t bytes[], const char* text);

	/// <summary>
	/// <para>zlib 形式で圧縮されたデータを展開する</para>
	/// <para>新しく配列を作って返す. 失敗したら空配列を返す</para>
	/// </summary>
	// TODO: もっと最適化できるか?
	static std::vector<std::uint8_t> InflateZlib(
		const std::vector<std::uint8_t>& compressedData, std::size_t expectedDecompressedSize);

	/// <summary>
	/// <para>PNG 画像ファイルを読み込み、生ビット配列として返す</para>
	/// <para>新しく配列を作って返す. 失敗したら空配列を返す</para>
	/// </summary>
	static std::vector<std::uint8_t> LoadPng(const std::string& path);

	/// <summary>
	/// <para>PNG 画像の生ビットを解析して、ピクセルデータを抽出する</para>
	/// <para>Z字の順番にピクセルカラーを返す. R,G,B,A の順番で各ピクセル1バイトずつ</para>
	/// <para>width, height に画像の幅・高さを格納する (失敗したら 0,0)</para>
	/// <para>ex:</para>
	/// <para>x=2,y=0 color=(11,22,33,44) の場合、戻り値について</para>
	/// <para>戻り値: [8]=11,[9]=22,[10]=33,[11]=44</para>
	/// </summary>
	/// <returns>ピクセルデータ (失敗したら空配列)</returns>
	static std::vector<std::uint8_t> ParseRawBitsPng(const std::vector<std::uint8_t>& rawBits, int& width, int& height);

	Texture TextureLoader::Load(const std::string& path)
	{
		const std::vector<std::uint8_t> rawBits = LoadPng(path);
		if (rawBits.empty())
			return Texture{};

		int width = 0;
		int height = 0;
		std::vector<std::uint8_t> rawData = ParseRawBitsPng(rawBits, width, height);
		if (rawData.empty())
			return Texture{};

		return Texture
		{
			.data = std::move(rawData),
			.width = width,
			.height = height,
			.sliceCount = 1,
		};
	}

	Texture TextureLoader::LoadAsArray(const std::vector<std::string>& paths)
	{
		if (paths.empty())
			return Texture{};

		const int sliceCount = static_cast<int>(paths.size());

		// 全画像のbit配列を結合する
		// 1枚目で reserve する
		std::vector<std::uint8_t> allData{};

		// 1枚目の画像のサイズを基準にする
		// 2枚目以降、同じサイズでなければ失敗
		int width = 0;
		int height = 0;

		for (const std::string& path : paths)
		{
			const Texture texture = Load(path);
			if (!texture.IsValid())
				return Texture{};

			// 1枚目
			if (allData.empty())
			{
				// 基準を決める
				width = texture.width;
				height = texture.height;

				allData.reserve(texture.GetSliceSize() * sliceCount);
			}
			// 2枚目以降
			else
			{
				// サイズが違うなら失敗
				if (texture.width != width || texture.height != height)
					return Texture{};
			}

			allData.insert(
				allData.end(),
				texture.data.begin(),
				texture.data.end()
			);
		}

		return Texture
		{
			.data = std::move(allData),
			.width = width,
			.height = height,
			.sliceCount = sliceCount,
		};
	}

	std::uint32_t ReinterpretAsBigEndianUint32(const std::uint8_t bytes[])
	{
		return
			(static_cast<std::uint32_t>(bytes[0]) << 24) |
			(static_cast<std::uint32_t>(bytes[1]) << 16) |
			(static_cast<std::uint32_t>(bytes[2]) << 8) |
			(static_cast<std::uint32_t>(bytes[3]));
	}

	bool CompareType(const std::uint8_t bytes[], const char* text)
	{
		return std::memcmp(bytes, text, 4) == 0;
	}

	std::vector<std::uint8_t> InflateZlib(
		const std::vector<std::uint8_t>& compressedData, std::size_t expectedDecompressedSize)
	{
		std::vector<std::uint8_t> output(expectedDecompressedSize);

		mz_ulong dstLen = static_cast<mz_ulong>(output.size());
		mz_ulong srcLen = static_cast<mz_ulong>(compressedData.size());

		int result = mz_uncompress(
			output.data(), &dstLen,
			compressedData.data(), srcLen
		);

		if (result != MZ_OK)
			return {};

		output.resize(dstLen);
		return output;
	}

	std::vector<std::uint8_t> LoadPng(const std::string& path)
	{
		std::ifstream ifs(path, std::ios::binary);
		if (!ifs)
			return {};

		// ファイルサイズ取得
		ifs.seekg(0, std::ios::end);
		const std::streamsize size = ifs.tellg();
		if (size <= 0)
			return {};

		ifs.seekg(0, std::ios::beg);

		std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
		if (!ifs.read(reinterpret_cast<char*>(data.data()), size))
			return {};

		return data;
	}

	std::vector<std::uint8_t> ParseRawBitsPng(const std::vector<std::uint8_t>& rawBits, int& width, int& height)
	{
		width = 0;
		height = 0;

		// 先頭8バイトのシグネチャ
		// 最初にこれをチェック. PNG 形式でなければ即座に失敗
		constexpr std::uint8_t Signatures[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
		if (rawBits.size() < 8) return {};
		for (size_t i = 0; i < 8; ++i)
		{
			if (rawBits[i] != Signatures[i])
				return {};
		}

		// チャンク群のビューを取得
		const std::span<const std::uint8_t> chunksSpan{ rawBits.data() + 8, rawBits.size() - 8 };

		// チャンクの生データを抽出し、連結して格納する (IDAT チャンクのみ)
		// 連結後のデータは圧縮されている
		std::vector<std::uint8_t> compressedData{}; // 後で reserve
		std::uint64_t offset = 0;
		// IHDR チャンクから取得する
		std::uint32_t fetchedWidth = 0;
		std::uint32_t fetchedHeight = 0;
		// ビッグエンディアンなことに注意しつつ、チャンクを読み取っていく
		// RGBA 8bit 固定で処理する
		while (true)
		{
			// dataLength + type を足しても範囲内であること
			if (offset + 8 > chunksSpan.size())
				return {};

			const std::uint32_t dataLength = ReinterpretAsBigEndianUint32(chunksSpan.data() + offset);
			offset += 4;

			// "IHDR", "IDAT", "IEND" など
			const std::uint8_t type[4]
			{
				chunksSpan[offset],
				chunksSpan[offset + 1],
				chunksSpan[offset + 2],
				chunksSpan[offset + 3],
			};
			offset += 4;

			// 最初: IHDR チャンクなので、画像の基本情報を読み取る
			if (CompareType(type, "IHDR"))
			{
				// データ長が 13 バイトであることを確認 (仕様上そうなっている)
				if (dataLength != 13) return {};

				// 画像情報を取得
				fetchedWidth = ReinterpretAsBigEndianUint32(chunksSpan.data() + offset);
				fetchedHeight = ReinterpretAsBigEndianUint32(chunksSpan.data() + offset + 4);
				// ビット深度が 8bit 固定!
				if (chunksSpan[offset + 8] != 8) return {};
				// RGBA 固定!
				if (chunksSpan[offset + 9] != 6) return {};

				// 以下は見ない
				// [10]: Compression (1 byte)
				// [11]: Filter (1 byte)
				// [12]: Interlace (1 byte)

				offset += dataLength + 4; // データ長 + CRC

				// 画像のサイズが分かったので、チャンクデータの格納先を reserve しておく
				// 圧縮済みのデータなので、テクセル数 x 4 バイト分で十分なはず
				compressedData.reserve(fetchedWidth * fetchedHeight * 4);

				continue;
			}

			// 最後: IEND チャンクまで来たので、終了
			if (CompareType(type, "IEND"))
			{
				// dataLength = 0
				offset += dataLength + 4; // 一応消費しておく
				break;
			}

			// データ長が範囲外でないかのチェック
			// データ長 + CRC を足しても範囲内であること
			if (offset + dataLength + 4 > chunksSpan.size())
				return {};

			// 一応、IDAT であるかのチェック
			if (!CompareType(type, "IDAT"))
			{
				// 未対応チャンクなので、処理失敗
				offset += dataLength + 4; // データ長 + CRC
				return {};
			}

			compressedData.insert(
				compressedData.end(),
				chunksSpan.data() + offset,
				chunksSpan.data() + offset + dataLength
			);
			offset += dataLength;

			// TODO: 本来は CRC を用いて検証するべきだけど、一旦スキップ
			const std::uint32_t crc = ReinterpretAsBigEndianUint32(chunksSpan.data() + offset);
			offset += 4;
		}

		// 圧縮データを展開する
		std::vector<std::uint8_t> decompressedData = InflateZlib(
			compressedData,
			static_cast<std::size_t>(fetchedHeight * (1 + fetchedWidth * 4)) // 各スキャンラインの先頭にフィルタバイトが入るので、その分を加える
		);

		// フィルターを処理して、最終的なピクセルデータを得る

		// 最終データ. 左上を (0, 0) とした時、
		// index = (y * width + x) * 4 + [R=0,G=1,B=2,A=3]
		std::vector<std::uint8_t> finalData(fetchedWidth * fetchedHeight * 4);
		// 各スキャンライン毎に処理していく
		const std::uint32_t widthSize = fetchedWidth * 4;
		for (std::uint32_t y = 0; y < fetchedHeight; ++y)
		{
			// スキャンラインの先頭ピクセルのインデックス
			// スキャンラインの先頭1バイトがフィルタタイプ
			const std::uint32_t rowStartIndex = y * (widthSize + 1);

			uint8_t filter = decompressedData[rowStartIndex];
			// 展開されたデータの、このスキャンラインにおけるフィルタを除去する
			switch (filter)
			{
			case 0: // None
			{
				// 何もしない
				break;
			}
			case 1: // Sub. 左隣のピクセルからの差分として保存されている
			{
				// スキャンラインのデータが始まるインデックス (フィルタバイトの次)
				const std::uint32_t rowDataStartIndex = rowStartIndex + 1;

				for (std::uint32_t x = 1; x < fetchedWidth; ++x)
				{
					const std::uint8_t leftR = decompressedData[rowDataStartIndex + (x - 1) * 4 + 0];
					const std::uint8_t leftG = decompressedData[rowDataStartIndex + (x - 1) * 4 + 1];
					const std::uint8_t leftB = decompressedData[rowDataStartIndex + (x - 1) * 4 + 2];
					const std::uint8_t leftA = decompressedData[rowDataStartIndex + (x - 1) * 4 + 3];

					decompressedData[rowDataStartIndex + x * 4 + 0] += leftR;
					decompressedData[rowDataStartIndex + x * 4 + 1] += leftG;
					decompressedData[rowDataStartIndex + x * 4 + 2] += leftB;
					decompressedData[rowDataStartIndex + x * 4 + 3] += leftA;
				}

				break;
			}
			case 2: // Up. 上のピクセルからの差分として保存されている
			{
				// スキャンラインのデータが始まるインデックス (フィルタバイトの次)
				const std::uint32_t rowDataStartIndex = rowStartIndex + 1;

				if (y > 0)
				{
					// 1行上のスキャンラインの、先頭ピクセルのインデックス
					const std::uint32_t upRowStartIndex = (y - 1) * (widthSize + 1);
					// 1行上のスキャンラインの、データが始まるインデックス (フィルタバイトの次)
					const std::uint32_t upRowDataStartIndex = upRowStartIndex + 1;

					for (std::uint32_t x = 0; x < fetchedWidth; ++x)
					{
						const std::uint8_t upR = decompressedData[upRowDataStartIndex + x * 4 + 0];
						const std::uint8_t upG = decompressedData[upRowDataStartIndex + x * 4 + 1];
						const std::uint8_t upB = decompressedData[upRowDataStartIndex + x * 4 + 2];
						const std::uint8_t upA = decompressedData[upRowDataStartIndex + x * 4 + 3];

						decompressedData[rowDataStartIndex + x * 4 + 0] += upR;
						decompressedData[rowDataStartIndex + x * 4 + 1] += upG;
						decompressedData[rowDataStartIndex + x * 4 + 2] += upB;
						decompressedData[rowDataStartIndex + x * 4 + 3] += upA;
					}
				}

				break;
			}
			default:
				break;
			}

			std::memcpy(
				finalData.data() + (y * widthSize),
				decompressedData.data() + (rowStartIndex + 1),
				widthSize
			);
		}

		width = static_cast<int>(fetchedWidth);
		height = static_cast<int>(fetchedHeight);
		return finalData;
	}
}
