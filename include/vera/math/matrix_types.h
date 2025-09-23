#pragma once

#include "../core/coredefs.h"
#include <glm/matrix.hpp>

VERA_NAMESPACE_BEGIN

typedef glm::mat<2, 2, float, glm::packed_highp> float2x2;
typedef glm::mat<2, 3, float, glm::packed_highp> float2x3;
typedef glm::mat<2, 4, float, glm::packed_highp> float2x4;
typedef glm::mat<3, 2, float, glm::packed_highp> float3x2;
typedef glm::mat<3, 3, float, glm::packed_highp> float3x3;
typedef glm::mat<3, 4, float, glm::packed_highp> float3x4;
typedef glm::mat<4, 2, float, glm::packed_highp> float4x2;
typedef glm::mat<4, 3, float, glm::packed_highp> float4x3;
typedef glm::mat<4, 4, float, glm::packed_highp> float4x4;

typedef glm::mat<2, 2, double, glm::packed_highp> double2x2;
typedef glm::mat<2, 3, double, glm::packed_highp> double2x3;
typedef glm::mat<2, 4, double, glm::packed_highp> double2x4;
typedef glm::mat<3, 2, double, glm::packed_highp> double3x2;
typedef glm::mat<3, 3, double, glm::packed_highp> double3x3;
typedef glm::mat<3, 4, double, glm::packed_highp> double3x4;
typedef glm::mat<4, 2, double, glm::packed_highp> double4x2;
typedef glm::mat<4, 3, double, glm::packed_highp> double4x3;
typedef glm::mat<4, 4, double, glm::packed_highp> double4x4;

VERA_NAMESPACE_END