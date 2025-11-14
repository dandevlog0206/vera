#pragma once

#include "vector_math.h"
#include <utility>

VERA_NAMESPACE_BEGIN
VERA_SDF_NAMESPACE_BEGIN

VERA_NODISCARD VERA_CONSTEXPR float circle(const float2& p, float r) VERA_NOEXCEPT
{
	return length(p) - r;
}

VERA_NODISCARD VERA_CONSTEXPR float roundBox(const float2& p, const float2& b, const float4& r) VERA_NOEXCEPT
{
	return 0.0;
}

VERA_NODISCARD VERA_CONSTEXPR float chamferBox(const float2& p, const float2& b) VERA_NOEXCEPT
{
	return 0.0;
}

VERA_SDF_NAMESPACE_END
VERA_NAMESPACE_END