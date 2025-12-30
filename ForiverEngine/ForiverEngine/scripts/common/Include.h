#pragma once

#include <cmath>
#include <string>

#include <vector>
#include <array>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include <algorithm>
#include <functional>
#include <utility>

#include <thread>
#include <mutex>
#include <atomic>

#include <iostream>
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>

#include "./Math/Include.h"
#include "./Component/Include.h"
#include "./Utils/Include.h"

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
