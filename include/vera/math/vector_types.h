#pragma once

#include "../core/coredefs.h"
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>
#include <glm/detail/type_half.hpp>

VERA_NAMESPACE_BEGIN

template <size_t dim, class T>
using vector_base = glm::vec<dim, T, glm::packed_highp>;

typedef glm::vec<2, float, glm::packed_highp> float2;
typedef glm::vec<3, float, glm::packed_highp> float3;
typedef glm::vec<4, float, glm::packed_highp> float4;

typedef glm::vec<2, double, glm::packed_highp> double2;
typedef glm::vec<3, double, glm::packed_highp> double3;
typedef glm::vec<4, double, glm::packed_highp> double4;

typedef glm::vec<2, bool, glm::packed_highp> bool2;
typedef glm::vec<3, bool, glm::packed_highp> bool3;
typedef glm::vec<4, bool, glm::packed_highp> bool4;

typedef glm::vec<2, int8_t, glm::packed_highp> char2;
typedef glm::vec<3, int8_t, glm::packed_highp> char3;
typedef glm::vec<4, int8_t, glm::packed_highp> char4;

typedef glm::vec<2, uint8_t, glm::packed_highp> uchar2;
typedef glm::vec<3, uint8_t, glm::packed_highp> uchar3;
typedef glm::vec<4, uint8_t, glm::packed_highp> uchar4;

typedef glm::vec<2, int16_t, glm::packed_highp> short2;
typedef glm::vec<3, int16_t, glm::packed_highp> short3;
typedef glm::vec<4, int16_t, glm::packed_highp> short4;

typedef glm::vec<2, uint16_t, glm::packed_highp> ushort2;
typedef glm::vec<3, uint16_t, glm::packed_highp> ushort3;
typedef glm::vec<4, uint16_t, glm::packed_highp> ushort4;

typedef glm::vec<2, int32_t, glm::packed_highp> int2;
typedef glm::vec<3, int32_t, glm::packed_highp> int3;
typedef glm::vec<4, int32_t, glm::packed_highp> int4;

typedef glm::vec<2, uint32_t, glm::packed_highp> uint2;
typedef glm::vec<3, uint32_t, glm::packed_highp> uint3;
typedef glm::vec<4, uint32_t, glm::packed_highp> uint4;

typedef glm::vec<2, int64_t, glm::packed_highp> long2;
typedef glm::vec<3, int64_t, glm::packed_highp> long3;
typedef glm::vec<4, int64_t, glm::packed_highp> long4;

typedef glm::vec<2, uint64_t, glm::packed_highp> ulong2;
typedef glm::vec<3, uint64_t, glm::packed_highp> ulong3;
typedef glm::vec<4, uint64_t, glm::packed_highp> ulong4;

VERA_NAMESPACE_END