#pragma once

#include "detail/vector2.h"
#include "detail/vector3.h"
#include "detail/vector4.h"

VERA_NAMESPACE_BEGIN

typedef vector_base<2, float, packed_highp> float2;
typedef vector_base<3, float, packed_highp> float3;
typedef vector_base<4, float, packed_highp> float4;

typedef vector_base<2, double, packed_highp> double2;
typedef vector_base<3, double, packed_highp> double3;
typedef vector_base<4, double, packed_highp> double4;

typedef vector_base<2, bool, packed_highp> bool2;
typedef vector_base<3, bool, packed_highp> bool3;
typedef vector_base<4, bool, packed_highp> bool4;

typedef vector_base<2, int8_t, packed_highp> char2;
typedef vector_base<3, int8_t, packed_highp> char3;
typedef vector_base<4, int8_t, packed_highp> char4;

typedef vector_base<2, uint8_t, packed_highp> uchar2;
typedef vector_base<3, uint8_t, packed_highp> uchar3;
typedef vector_base<4, uint8_t, packed_highp> uchar4;

typedef vector_base<2, int16_t, packed_highp> short2;
typedef vector_base<3, int16_t, packed_highp> short3;
typedef vector_base<4, int16_t, packed_highp> short4;

typedef vector_base<2, uint16_t, packed_highp> ushort2;
typedef vector_base<3, uint16_t, packed_highp> ushort3;
typedef vector_base<4, uint16_t, packed_highp> ushort4;

typedef vector_base<2, int32_t, packed_highp> int2;
typedef vector_base<3, int32_t, packed_highp> int3;
typedef vector_base<4, int32_t, packed_highp> int4;

typedef vector_base<2, uint32_t, packed_highp> uint2;
typedef vector_base<3, uint32_t, packed_highp> uint3;
typedef vector_base<4, uint32_t, packed_highp> uint4;

typedef vector_base<2, int64_t, packed_highp> long2;
typedef vector_base<3, int64_t, packed_highp> long3;
typedef vector_base<4, int64_t, packed_highp> long4;

typedef vector_base<2, uint64_t, packed_highp> ulong2;
typedef vector_base<3, uint64_t, packed_highp> ulong3;
typedef vector_base<4, uint64_t, packed_highp> ulong4;

VERA_NAMESPACE_END