#pragma once

#include "../core/coredefs.h"

VERA_NAMESPACE_BEGIN

typedef float radian;

VERA_CONSTEXPR radian to_radian(float degree) VERA_NOEXCEPT
{
	return static_cast<radian>(degree * (3.14159265358979 / 180.f));
}

VERA_CONSTEXPR float to_deg(radian radian) VERA_NOEXCEPT
{
	return static_cast<float>(radian * (180.f / 3.14159265358979));
}

VERA_NAMESPACE_END