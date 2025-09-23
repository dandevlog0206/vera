#pragma once

#include "../math/vector_types.h"

VERA_NAMESPACE_BEGIN

template <size_t dim, class T>
struct extent_base {
	static_assert("unsupported dimension");
};

template <class T>
struct extent_base<2, T> {
	VERA_CONSTEXPR extent_base() VERA_NOEXCEPT :
		width(), height() {}

	VERA_CONSTEXPR extent_base(T width, T height) VERA_NOEXCEPT :
		width(width), height(height) {}

	VERA_NODISCARD VERA_CONSTEXPR T area() const VERA_NOEXCEPT {
		return width * height;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const extent_base& rhs) const VERA_NOEXCEPT {
		return width == rhs.width && height == rhs.height;
	}

	VERA_NODISCARD VERA_CONSTEXPR operator uint2() const VERA_NOEXCEPT {
		return { width, height };
	}

	T width;
	T height;
};

template <class T>
struct extent_base<3, T> {
	VERA_CONSTEXPR extent_base() VERA_NOEXCEPT :
		width(), height(), depth() {}

	VERA_CONSTEXPR extent_base(T width, T height, T depth) VERA_NOEXCEPT :
		width(width), height(height), depth(depth) {}

	VERA_NODISCARD VERA_CONSTEXPR T volume() const VERA_NOEXCEPT {
		return width * height * depth;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const extent_base& rhs) const VERA_NOEXCEPT {
		return width == rhs.width && height == rhs.height && depth == rhs.depth;
	}

	VERA_NODISCARD VERA_CONSTEXPR operator uint3() const VERA_NOEXCEPT {
		return { width, height, depth };
	}
	
	T width;
	T height;
	T depth;
};

typedef extent_base<2, uint32_t> extent2d;
typedef extent_base<3, uint32_t> extent3d;

VERA_NAMESPACE_END