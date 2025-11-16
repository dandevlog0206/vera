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

typedef matrix_base<2, 2, row_major, float, packed_highp> float2x2;
typedef matrix_base<2, 3, row_major, float, packed_highp> float2x3;
typedef matrix_base<2, 4, row_major, float, packed_highp> float2x4;
typedef matrix_base<3, 2, row_major, float, packed_highp> float3x2;
typedef matrix_base<3, 3, row_major, float, packed_highp> float3x3;
typedef matrix_base<3, 4, row_major, float, packed_highp> float3x4;
typedef matrix_base<4, 2, row_major, float, packed_highp> float4x2;
typedef matrix_base<4, 3, row_major, float, packed_highp> float4x3;
typedef matrix_base<4, 4, row_major, float, packed_highp> float4x4;

typedef matrix_base<2, 2, row_major, double, packed_highp> double2x2;
typedef matrix_base<2, 3, row_major, double, packed_highp> double2x3;
typedef matrix_base<2, 4, row_major, double, packed_highp> double2x4;
typedef matrix_base<3, 2, row_major, double, packed_highp> double3x2;
typedef matrix_base<3, 3, row_major, double, packed_highp> double3x3;
typedef matrix_base<3, 4, row_major, double, packed_highp> double3x4;
typedef matrix_base<4, 2, row_major, double, packed_highp> double4x2;
typedef matrix_base<4, 3, row_major, double, packed_highp> double4x3;
typedef matrix_base<4, 4, row_major, double, packed_highp> double4x4;

typedef matrix_base<2, 2, row_major, float, packed_highp> rfloat2x2;
typedef matrix_base<2, 3, row_major, float, packed_highp> rfloat2x3;
typedef matrix_base<2, 4, row_major, float, packed_highp> rfloat2x4;
typedef matrix_base<3, 2, row_major, float, packed_highp> rfloat3x2;
typedef matrix_base<3, 3, row_major, float, packed_highp> rfloat3x3;
typedef matrix_base<3, 4, row_major, float, packed_highp> rfloat3x4;
typedef matrix_base<4, 2, row_major, float, packed_highp> rfloat4x2;
typedef matrix_base<4, 3, row_major, float, packed_highp> rfloat4x3;
typedef matrix_base<4, 4, row_major, float, packed_highp> rfloat4x4;

typedef matrix_base<2, 2, row_major, double, packed_highp> rdouble2x2;
typedef matrix_base<2, 3, row_major, double, packed_highp> rdouble2x3;
typedef matrix_base<2, 4, row_major, double, packed_highp> rdouble2x4;
typedef matrix_base<3, 2, row_major, double, packed_highp> rdouble3x2;
typedef matrix_base<3, 3, row_major, double, packed_highp> rdouble3x3;
typedef matrix_base<3, 4, row_major, double, packed_highp> rdouble3x4;
typedef matrix_base<4, 2, row_major, double, packed_highp> rdouble4x2;
typedef matrix_base<4, 3, row_major, double, packed_highp> rdouble4x3;
typedef matrix_base<4, 4, row_major, double, packed_highp> rdouble4x4;

typedef matrix_base<2, 2, col_major, float, packed_highp> cfloat2x2;
typedef matrix_base<2, 3, col_major, float, packed_highp> cfloat2x3;
typedef matrix_base<2, 4, col_major, float, packed_highp> cfloat2x4;
typedef matrix_base<3, 2, col_major, float, packed_highp> cfloat3x2;
typedef matrix_base<3, 3, col_major, float, packed_highp> cfloat3x3;
typedef matrix_base<3, 4, col_major, float, packed_highp> cfloat3x4;
typedef matrix_base<4, 2, col_major, float, packed_highp> cfloat4x2;
typedef matrix_base<4, 3, col_major, float, packed_highp> cfloat4x3;
typedef matrix_base<4, 4, col_major, float, packed_highp> cfloat4x4;

typedef matrix_base<2, 2, col_major, double, packed_highp> cdouble2x2;
typedef matrix_base<2, 3, col_major, double, packed_highp> cdouble2x3;
typedef matrix_base<2, 4, col_major, double, packed_highp> cdouble2x4;
typedef matrix_base<3, 2, col_major, double, packed_highp> cdouble3x2;
typedef matrix_base<3, 3, col_major, double, packed_highp> cdouble3x3;
typedef matrix_base<3, 4, col_major, double, packed_highp> cdouble3x4;
typedef matrix_base<4, 2, col_major, double, packed_highp> cdouble4x2;
typedef matrix_base<4, 3, col_major, double, packed_highp> cdouble4x3;
typedef matrix_base<4, 4, col_major, double, packed_highp> cdouble4x4;

VERA_NAMESPACE_END