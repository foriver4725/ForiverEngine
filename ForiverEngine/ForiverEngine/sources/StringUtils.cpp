#include "../headers/StringUtils.h"

#include <windows.h>

namespace ForiverEngine
{
	std::wstring StringUtils::UTF8ToUTF16(const std::string& utf8)
	{
		// 必要な UTF-16 バッファサイズを取得 (終端文字含む)
		int sizeNeeded = MultiByteToWideChar(
			CP_UTF8,            // UTF-8 を扱う
			0,                  // フラグ（0でOK）
			utf8.c_str(),       // 入力するUTF-8文字列
			-1,                 // 終端文字まで読む
			nullptr,            // 出力先なし → サイズのみ取得
			0
		);

		if (sizeNeeded <= 0)
			return L"";

		std::wstring wide(sizeNeeded, 0);

		// 実際に UTF-16 へ変換
		MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), sizeNeeded);

		// 末尾の '\0' は std::wstring で不要なので削除
		wide.resize(sizeNeeded - 1);

		return wide;
	}

	std::string StringUtils::UTF16ToUTF8(const std::wstring& utf16)
	{
		// 必要な UTF-8 バッファサイズを取得
		int sizeNeeded = WideCharToMultiByte(
			CP_UTF8,            // UTF-8 出力
			0,                  // フラグ
			utf16.c_str(),      // 入力 UTF-16
			-1,                 // 終端まで
			nullptr,            // サイズのみ
			0,
			nullptr,
			nullptr
		);

		if (sizeNeeded <= 0)
			return "";

		std::string utf8(sizeNeeded, 0);

		// 実際に UTF-8 へ変換
		WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, utf8.data(), sizeNeeded, nullptr, nullptr);

		// 末尾の '\0' は削る
		utf8.resize(sizeNeeded - 1);

		return utf8;
	}
}
