#pragma once

#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

VERA_NAMESPACE_BEGIN

enum MathMatrixOrder
{
	row_major,
	col_major,

	default_major = row_major
};

// Helper: dependent false to delay static_assert until instantiation
template <typename>
inline constexpr bool always_false_v = false;

template <MathDimType MajorDim, MathDimType MinorDim, MathMatrixOrder Major, class T, MathQualifier Q>
class matrix_base
{
	static_assert(always_false_v<T>, "unsupported matrix type");
};

VERA_NAMESPACE_END
