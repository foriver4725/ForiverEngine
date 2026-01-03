#pragma once

#include <scripts/common/Include.h>
#include <scripts/helper/Include.h>
#include <scripts/component/Include.h>
#include <scripts/gameFlow/Include.h>

namespace ForiverEngine
{
	namespace Test
	{
		template<typename T>
		inline void Assert(const T& value, const T& expected, const std::string& fileName, int lineNumber)
		{
			const std::string errorMessage = std::format("Test failed\nValue: {}\nExpected: {}", value, expected);
			_wassert(
				StringUtils::UTF8ToUTF16(errorMessage).c_str(),
				StringUtils::UTF8ToUTF16(fileName).c_str(),
				static_cast<unsigned>(lineNumber)
			);
		}

		// std::format できる型に変換してから渡すこと
#define ASSERT(value, expected) ForiverEngine::Test::Assert((value), (expected), __FILE__, __LINE__);

		// 基本
#define  eq(value, expected)    { if ((value) != (expected)) ASSERT((value), (expected)); }
#define neq(value, expected)    { if ((value) == (expected)) ASSERT((value), (expected)); }

		// std::string のコンストラクタに渡す
#define  eqs(value, expected)   { if ((value) != (expected)) ASSERT((std::string(value)), (std::string(expected))); }
#define neqs(value, expected)   { if ((value) == (expected)) ASSERT((std::string(value)), (std::string(expected))); }

		// 列挙型 (static_cast<int> する)
#define  eqen(value, expected)  { if ((value) != (expected)) ASSERT((static_cast<int>(value)), (static_cast<int>(expected))); }
#define neqen(value, expected)  { if ((value) == (expected)) ASSERT((static_cast<int>(value)), (static_cast<int>(expected))); }

		// 線形代数 (Linear Algebra)
#define  eqla(value, expected)  { if ((value) != (expected)) ASSERT((ToString(value)), (ToString(expected))); }
#define neqla(value, expected)  { if ((value) == (expected)) ASSERT((ToString(value)), (ToString(expected))); }

		// 自作オブジェクト (ToString メソッドを持つ)
#define  eqobj(value, expected) { if ((value) != (expected)) ASSERT((value.ToString()), (expected.ToString())); }
#define neqobj(value, expected) { if ((value) == (expected)) ASSERT((value.ToString()), (expected.ToString())); }
	}
}
