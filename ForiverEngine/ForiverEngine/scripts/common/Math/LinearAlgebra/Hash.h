#pragma once

#include <functional>

#include "./headers/Lattice2.h"
#include "./headers/Lattice3.h"
#include "./headers/Lattice4.h"

namespace ForiverEngine
{
	namespace HashDetail
	{
		template<typename T>
		inline void HashCombine(std::size_t& seed, const T& value) noexcept
		{
			const std::size_t h = std::hash<T>{}(value);
			seed ^= h + static_cast<std::size_t>(0x9e3779b9) + (seed << 6) + (seed >> 2);
		}

		template<typename T, typename... Rest>
		inline std::size_t MakeHash(const T& value, const Rest&... rest) noexcept
		{
			std::size_t seed = 0;
			HashCombine(seed, value);
			(HashCombine(seed, rest), ...);
			return seed;
		}
	}
}

// 整数ベクトルをハッシュ化するための std::hash の特殊化
namespace std
{
	template<>
	struct hash<ForiverEngine::Lattice2>
	{
		std::size_t operator()(const ForiverEngine::Lattice2& v) const noexcept
		{
			return ForiverEngine::HashDetail::MakeHash(v.x, v.y);
		}
	};

	template<>
	struct hash<ForiverEngine::Lattice3>
	{
		std::size_t operator()(const ForiverEngine::Lattice3& v) const noexcept
		{
			return ForiverEngine::HashDetail::MakeHash(v.x, v.y, v.z);
		}
	};

	template<>
	struct hash<ForiverEngine::Lattice4>
	{
		std::size_t operator()(const ForiverEngine::Lattice4& v) const noexcept
		{
			return ForiverEngine::HashDetail::MakeHash(v.x, v.y, v.z, v.w);
		}
	};
}