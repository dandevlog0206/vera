#pragma once

#include "../core/coredefs.h"

VERA_NAMESPACE_BEGIN

template <typename T>
void hash_combine(size_t& seed, const T& val) {
	if constexpr (sizeof(size_t) == 4)
		seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	else if constexpr (sizeof(size_t) == 8)
		seed ^= std::hash<T>()(val) + 0x9e3779b97f4a7c15LLU + (seed << 12) + (seed >> 4);
}

VERA_NAMESPACE_END
