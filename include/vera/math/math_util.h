#pragma once

#include "../core/coredefs.h"

VERA_NAMESPACE_BEGIN

template <class T>
VERA_NODISCARD VERA_CONSTEXPR bool in_range(const T& value, const T& min, const T& max) VERA_NOEXCEPT
{
	return (min <= value) && (value <= max);
}

template <class T, class Res>
VERA_NODISCARD VERA_CONSTEXPR Res lerp(const Res& a, const Res& b, const T& t) VERA_NOEXCEPT
{
	return a + static_cast<Res>((b - a) * t);
}

template <class T, class Res>
VERA_NODISCARD VERA_CONSTEXPR T unlerp(const Res& a, const Res& b, const Res& value) VERA_NOEXCEPT
{
	return static_cast<T>((value - a) / (b - a));
}


VERA_NAMESPACE_END