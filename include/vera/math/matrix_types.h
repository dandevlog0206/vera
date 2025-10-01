#pragma once

#include "detail/matrix2x2.h"
#include "detail/matrix2x3.h"
#include "detail/matrix2x4.h"
#include "detail/matrix3x2.h"
#include "detail/matrix3x3.h"
#include "detail/matrix3x4.h"
#include "detail/matrix4x2.h"
#include "detail/matrix4x3.h"
#include "detail/matrix4x4.h"

VERA_NAMESPACE_BEGIN

typedef matrix_base<2, 2, float, packed_highp> float2x2;
typedef matrix_base<2, 3, float, packed_highp> float2x3;
typedef matrix_base<2, 4, float, packed_highp> float2x4;
typedef matrix_base<3, 2, float, packed_highp> float3x2;
typedef matrix_base<3, 3, float, packed_highp> float3x3;
typedef matrix_base<3, 4, float, packed_highp> float3x4;
typedef matrix_base<4, 2, float, packed_highp> float4x2;
typedef matrix_base<4, 3, float, packed_highp> float4x3;
typedef matrix_base<4, 4, float, packed_highp> float4x4;

typedef matrix_base<2, 2, double, packed_highp> double2x2;
typedef matrix_base<2, 3, double, packed_highp> double2x3;
typedef matrix_base<2, 4, double, packed_highp> double2x4;
typedef matrix_base<3, 2, double, packed_highp> double3x2;
typedef matrix_base<3, 3, double, packed_highp> double3x3;
typedef matrix_base<3, 4, double, packed_highp> double3x4;
typedef matrix_base<4, 2, double, packed_highp> double4x2;
typedef matrix_base<4, 3, double, packed_highp> double4x3;
typedef matrix_base<4, 4, double, packed_highp> double4x4;

VERA_NAMESPACE_END