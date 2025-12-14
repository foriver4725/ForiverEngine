#pragma once

#include <cmath>
#include <string>

#include <vector>
#include <array>
#include <map>
#include <tuple>

#include <algorithm>
#include <functional>

#include <iostream>
#define NOMINMAX
#include <Windows.h>

#include "./Math/Include.h"
#include "./Component/Include.h"
#include "./StringUtils.h"
#include "./Noise.h"

// コンストラクタとデストラクタ、代入演算子を削除するマクロ
// - デフォルトコンストラクタ
// - コピーコンストラクタ
// - ムーブコンストラクタ
// - デストラクタ
// - コピー代入演算子
// - ムーブ代入演算子
#define DELETE_DEFAULT_METHODS(ClassName) \
\
    ClassName() = delete; \
    ClassName(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
\
    ~ClassName() = delete; \
\
    ClassName& operator=(const ClassName&) = delete; \
    ClassName& operator=(ClassName&&) = delete; \
