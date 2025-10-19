#pragma once

#include "vector_math.h"
#include <utility>

VERA_NAMESPACE_BEGIN
VERA_SDF_NAMESPACE_BEGIN

VERA_NODISCARD VERA_CONSTEXPR float circle(const float2& p, float r) VERA_NOEXCEPT
{
	return length(p) - r;
}

VERA_NODISCARD VERA_CONSTEXPR float round_box(const float2& p, const float2& b, const float4& r) VERA_NOEXCEPT
{

}

VERA_NODISCARD VERA_CONSTEXPR float box(const float2& p, const float2& b) VERA_NOEXCEPT
{

}

VERA_SDF_NAMESPACE_END
VERA_NAMESPACE_END