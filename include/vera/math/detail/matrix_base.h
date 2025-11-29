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

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<2, 2, row_major, T, Q> to_row_major(
	const matrix_base<2, 2, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0],
		mat[0][1], mat[1][1]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<2, 3, row_major, T, Q> to_row_major(
	const matrix_base<3, 2, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0], mat[2][0],
		mat[0][1], mat[1][1], mat[2][1]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<2, 4, row_major, T, Q> to_row_major(
	const matrix_base<4, 2, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0], mat[2][0],  mat[3][0],
		mat[0][1], mat[1][1], mat[2][1],  mat[3][1]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<3, 2, row_major, T, Q> to_row_major(
	const matrix_base<2, 3, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0],
		mat[0][1], mat[1][1],
		mat[0][2], mat[1][2]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<3, 3, row_major, T, Q> to_row_major(
	const matrix_base<4, 3, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0], mat[2][0],
		mat[0][1], mat[1][1], mat[2][1],
		mat[0][2], mat[1][2], mat[2][2]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<3, 4, row_major, T, Q> to_row_major(
	const matrix_base<4, 3, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0], mat[2][0],  mat[3][0],
		mat[0][1], mat[1][1], mat[2][1],  mat[3][1],
		mat[0][2], mat[1][2], mat[2][2],  mat[3][2]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 2, row_major, T, Q> to_row_major(
	const matrix_base<2, 4, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0],
		mat[0][1], mat[1][1],
		mat[0][2], mat[1][2],
		mat[0][3], mat[1][3]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 3, row_major, T, Q> to_row_major(
	const matrix_base<3, 4, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0], mat[2][0],
		mat[0][1], mat[1][1], mat[2][1],
		mat[0][2], mat[1][2], mat[2][2],
		mat[0][3], mat[1][3], mat[2][3]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, row_major, T, Q> to_row_major(
	const matrix_base<4, 4, col_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[1][0], mat[2][0], mat[3][0],
		mat[0][1], mat[1][1], mat[2][1], mat[3][1],
		mat[0][2], mat[1][2], mat[2][2], mat[3][2],
		mat[0][3], mat[1][3], mat[2][3], mat[3][3]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<2, 2, col_major, T, Q> to_col_major(
	const matrix_base<2, 2, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1],
		mat[1][0], mat[1][1]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<3, 2, col_major, T, Q> to_col_major(
	const matrix_base<2, 3, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1], mat[0][2],
		mat[1][0], mat[1][1], mat[1][2]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 2, col_major, T, Q> to_col_major(
	const matrix_base<2, 4, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<2, 3, col_major, T, Q> to_col_major(
	const matrix_base<3, 2, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1],
		mat[1][0], mat[1][1],
		mat[2][0], mat[2][1]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<3, 3, col_major, T, Q> to_col_major(
	const matrix_base<3, 3, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1], mat[0][2],
		mat[1][0], mat[1][1], mat[1][2],
		mat[2][0], mat[2][1], mat[2][2]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 3, col_major, T, Q> to_col_major(
	const matrix_base<3, 4, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3],
		mat[2][0], mat[2][1], mat[2][2], mat[2][3]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<2, 4, col_major, T, Q> to_col_major(
	const matrix_base<4, 2, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1],
		mat[1][0], mat[1][1],
		mat[2][0], mat[2][1],
		mat[3][0], mat[3][1]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<3, 4, col_major, T, Q> to_col_major(
	const matrix_base<4, 3, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1], mat[0][2],
		mat[1][0], mat[1][1], mat[1][2],
		mat[2][0], mat[2][1], mat[2][2],
		mat[3][0], mat[3][1], mat[3][2]
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, col_major, T, Q> to_col_major(
	const matrix_base<4, 4, row_major, T, Q>& mat) VERA_NOEXCEPT
{
	return {
		mat[0][0], mat[0][1], mat[0][2], mat[0][3],
		mat[1][0], mat[1][1], mat[1][2], mat[1][3],
		mat[2][0], mat[2][1], mat[2][2], mat[2][3],
		mat[3][0], mat[3][1], mat[3][2], mat[3][3]
	};
}

VERA_NAMESPACE_END
