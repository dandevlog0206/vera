#pragma once

#include "../core/coredefs.h"
#include <type_traits>

VERA_NAMESPACE_BEGIN

typedef size_t hash_t;

// A hash combine function based on boost::hash_combine
template <typename T>
void hash_combine(hash_t& seed, const T& val)
{
	if constexpr (sizeof(hash_t) == 4)
		seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	else if constexpr (sizeof(hash_t) == 8)
		seed ^= std::hash<T>()(val) + 0x9e3779b97f4a7c15LLU + (seed << 12) + (seed >> 4);
}

// A hash combine function for unordered elements
void hash_unordered(hash_t& seed, hash_t hash)
{
	if constexpr (sizeof(hash_t) == 4)
		seed ^= (hash * 0x9e3779b9);
	else if constexpr (sizeof(hash_t) == 8)
		seed ^= (hash * 0x9e3779b97f4a7c15LLU);
}

VERA_NAMESPACE_END
