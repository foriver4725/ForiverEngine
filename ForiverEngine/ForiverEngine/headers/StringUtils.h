#pragma once

#include <string>

namespace ForiverEngine
{
	class StringUtils final
	{
	public:
		StringUtils() = delete;
		~StringUtils() = delete;

		/// <summary>
		/// UTF-8 (std::string) → UTF-16 (std::wstring) に変換する
		/// </summary>
		static std::wstring UTF8ToUTF16(const std::string& utf8);

		/// <summary>
		/// UTF-16 (std::wstring) → UTF-8 (std::string) に変換する
		/// </summary>
		static std::string UTF16ToUTF8(const std::wstring& utf16);
	};
}
