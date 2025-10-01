#pragma once

#include "../../core/coredefs.h"
#include "config.h"
#include <cstdint>

#define VEC2_SWIZZLE(v, x, y) decltype(v)::vec2_type((v).x, (v).y)
#define VEC3_SWIZZLE(v, x, y, z)
#define VEC4_SWIZZLE(v, x, y, z, w)

VERA_NAMESPACE_BEGIN

enum MathQualifier VERA_ENUM
{
	packed_highp,                    // Typed data is tightly packed in memory and operations are executed with high precision in term of ULPs
	packed_mediump,                  // Typed data is tightly packed in memory  and operations are executed with medium precision in term of ULPs for higher performance
	packed_lowp,                     // Typed data is tightly packed in memory  and operations are executed with low precision in term of ULPs to maximize performance
	packed         = packed_highp,   // By default packed qualifier is also high precision

#ifdef VERA_VECTOR_USE_SIMD
	aligned_highp,                   // Typed data is aligned in memory allowing SIMD optimizations and operations are executed with high precision in term of ULPs
	aligned_mediump,                 // Typed data is aligned in memory allowing SIMD optimizations and operations are executed with high precision in term of ULPs for higher performance
	aligned_lowp,                    // Typed data is aligned in memory allowing SIMD optimizations and operations are executed with high precision in term of ULPs to maximize performance
	aligned = aligned_highp,         // By default aligned qualifier is also high precision
#endif

	highp          = packed_highp,   // By default highp qualifier is also packed
	mediump        = packed_mediump, // By default mediump qualifier is also packed
	lowp           = packed_lowp,    // By default lowp qualifier is also packed
};

typedef unsigned int MathDimType;

template <MathDimType Dim, class T, MathQualifier Q>
class vector_base
{
	static_assert("unsupported vector type");
};

VERA_NAMESPACE_END
