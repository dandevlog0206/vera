#pragma once

#include "vector_math.h"

VERA_NAMESPACE_BEGIN
VERA_SDF_NAMESPACE_BEGIN

VERA_NODISCARD VERA_CONSTEXPR float sphere(const float3& p, float r) VERA_NOEXCEPT
{
	return length(p) - r;
}

VERA_SDF_NAMESPACE_END
VERA_NAMESPACE_END
