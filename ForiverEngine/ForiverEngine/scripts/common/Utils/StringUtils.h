#pragma once

#include <string>
#include <string_view>
#include <limits>
// TODO: Windows 以外の環境でも使えるようにしたい
#define NOMINMAX
#include <Windows.h>

#include "scripts/common/IncludeInternal.h"

namespace ForiverEngine
{
	class StringUtils final
	{
	public:
		DELETE_DEFAULT_METHODS(StringUtils);

		/// <summary>
		/// char -> wchar に変換する
		/// </summary>
		/// <param name="utf8">入力の Char 配列. UTF8 として解釈される</param>
		/// <returns>変換された UTF16 文字列 (Wide Char. Windows 環境). 失敗したら空文字列</returns>
		static std::wstring UTF8ToUTF16(std::string_view utf8)
		{
			if (utf8.empty())
				return L"";

			// Windows APIの長さ引数はint
			const std::size_t sizeRaw = utf8.size();
			if (sizeRaw > static_cast<std::size_t>(std::numeric_limits<int>::max()))
				return L"";

			const int size = static_cast<int>(sizeRaw);

			// 必要な UTF-16 バッファサイズを取得
			const int sizeNeeded = MultiByteToWideChar(
				CP_UTF8,              // 入力文字は UTF-8 として解釈する
				MB_ERR_INVALID_CHARS, // 無効な文字があったらエラーにする
				utf8.data(),          // 入力する UTF-8 文字列
				size,                 // \0 の存在が保証されないので、サイズを直指定する
				nullptr,              // 出力先なし → サイズのみ取得
				0
			);

			if (sizeNeeded <= 0)
				return L"";

			std::wstring utf16(sizeNeeded, L'\0');

			// 実際に UTF-16 へ変換
			const int written = MultiByteToWideChar(
				CP_UTF8,              // 入力文字は UTF-8 として解釈する
				MB_ERR_INVALID_CHARS, // 無効な文字があったらエラーにする
				utf8.data(),          // 入力する UTF-8 文字列
				size,                 // \0 の存在が保証されないので、サイズを直指定する
				utf16.data(),         // 出力先の Wide Char バッファ
				sizeNeeded            // 出力先のバッファサイズ
			);

			if (written <= 0)
				return  L"";

			return utf16;
		}

		/// <summary>
		/// wchar -> char に変換する
		/// </summary>
		/// <param name="utf16">入力の Wide Char 配列. UTF16 として解釈される</param>
		/// <returns>変換された UTF8 文字列 (Char). 失敗したら空文字列</returns>
		static std::string UTF16ToUTF8(std::wstring_view utf16)
		{
			if (utf16.empty())
				return "";

			// Windows APIの長さ引数はint
			const std::size_t sizeRaw = utf16.size();
			if (sizeRaw > static_cast<std::size_t>(std::numeric_limits<int>::max()))
				return "";

			const int size = static_cast<int>(sizeRaw);

			// 必要な UTF-8 バッファサイズを取得
			const int sizeNeeded = WideCharToMultiByte(
				CP_UTF8,              // 出力文字は UTF-8 として解釈する
				WC_ERR_INVALID_CHARS, // 無効な文字があったらエラーにする
				utf16.data(),         // 入力する UTF-16 文字列
				size,                 // \0 の存在が保証されないので、サイズを直指定する
				nullptr,              // 出力先なし → サイズのみ取得
				0,
				nullptr,
				nullptr
			);

			if (sizeNeeded <= 0)
				return "";

			std::string utf8(sizeNeeded, '\0');

			// 実際に UTF-8 へ変換
			const int written = WideCharToMultiByte(
				CP_UTF8,              // 出力文字は UTF-8 として解釈する
				WC_ERR_INVALID_CHARS, // 無効な文字があったらエラーにする
				utf16.data(),         // 入力する UTF-16 文字列
				size,                 // \0 の存在が保証されないので、サイズを直指定する
				utf8.data(),          // 出力先の Char バッファ
				sizeNeeded,           // 出力先のバッファサイズ
				nullptr,
				nullptr
			);

			if (written <= 0)
				return  "";

			return utf8;
		}
	};
}
