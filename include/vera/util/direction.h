#pragma once

#include "../core/coredefs.h"
#include <cstdint>

VERA_NAMESPACE_BEGIN

enum class Direction VERA_ENUM
{
	Up    = 0,
	Right = 1,
	Down  = 2,
	Left  = 3
};

enum class Rotation VERA_ENUM
{
	_0Deg   = 0,
	_90Deg  = 1,
	_180Deg = 2,
	_270Deg = 3,
	CW      = _90Deg,
	OPP     = 180,
	CCW     = _270Deg,
};

VERA_INLINE Direction rotate(Direction dir, Rotation rot)
{
	return static_cast<Direction>((static_cast<uint32_t>(dir) + static_cast<uint32_t>(rot)) & 0x3);
}

VERA_INLINE Direction rotate_cw(Direction dir)
{
	return static_cast<Direction>((static_cast<uint32_t>(dir) + 1) & 0x3);
}

VERA_INLINE Direction rotate_cw_n(Direction dir, uint32_t n)
{
	return static_cast<Direction>((static_cast<uint32_t>(dir) + n) & 0x3);
}

VERA_INLINE Direction rotate_ccw(Direction dir)
{
	return static_cast<Direction>((static_cast<uint32_t>(dir) + 3) & 0x3);
}

VERA_INLINE Direction rotate_ccw_n(Direction dir, uint32_t n)
{
	return static_cast<Direction>((static_cast<uint32_t>(dir) + 3 - (n & 0x3)) & 0x3);
}

VERA_NAMESPACE_END