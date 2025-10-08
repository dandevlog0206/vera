#pragma once

#include "matrix_base.h"

VERA_NAMESPACE_BEGIN

#ifdef VERA_MATRIX_GL_LAYOUT

template <class T, MathQualifier Q>
class matrix_base<3, 3, T, Q>
{
public:
	static VERA_CONSTEXPR MathDimType col_size = 3;
	static VERA_CONSTEXPR MathDimType row_size = 3;

	using col_type    = vector_base<3, T, Q>;
	using row_type    = vector_base<3, T, Q>;
	using vec2_type   = vector_base<2, T, Q>;
	using vec3_type   = vector_base<3, T, Q>;
	using vec4_type   = vector_base<4, T, Q>;
	using mat2x2_type = matrix_base<2, 2, T, Q>;
	using mat2x3_type = matrix_base<2, 3, T, Q>;
	using mat2x4_type = matrix_base<2, 4, T, Q>;
	using mat3x2_type = matrix_base<3, 2, T, Q>;
	using mat3x3_type = matrix_base<3, 3, T, Q>;
	using mat3x4_type = matrix_base<3, 4, T, Q>;
	using mat4x2_type = matrix_base<4, 2, T, Q>;
	using mat4x3_type = matrix_base<4, 3, T, Q>;
	using mat4x4_type = matrix_base<4, 4, T, Q>;
	using this_type   = matrix_base<3, 3, T, Q>;

	VERA_CONSTEXPR matrix_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR matrix_base(T diagonal) :
		col{
			col_type(diagonal, 0, 0),
			col_type(0, diagonal, 0),
			col_type(0, 0, diagonal) } {}

	VERA_CONSTEXPR matrix_base(
		T m00, T m01, T m02,
		T m10, T m11, T m12,
		T m20, T m21, T m22
	) VERA_NOEXCEPT :
		col{
			col_type(m00, m01, m02),
			col_type(m10, m11, m12),
			col_type(m20, m21, m22) } {}

	VERA_CONSTEXPR matrix_base(
		const col_type& col0,
		const col_type& col1,
		const col_type& col2
	) VERA_NOEXCEPT :
		col{
			col0,
			col1,
			col2 } {}

	VERA_CONSTEXPR matrix_base(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR matrix_base& operator=(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type transpose() const VERA_NOEXCEPT
	{
		return {
			col[0][0], col[1][0], col[2][0],
			col[0][1], col[1][1], col[2][1],
			col[0][2], col[1][2], col[2][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR T det() const VERA_NOEXCEPT
	{
		return
			col[0][0] * (col[1][1] * col[2][2] - col[1][2] * col[2][1]) -
			col[0][1] * (col[1][0] * col[2][2] - col[1][2] * col[2][0]) +
			col[0][2] * (col[1][0] * col[2][1] - col[1][1] * col[2][0]);
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type inv() const VERA_NOEXCEPT
	{
		const T m[9] = {
			col[0][0], col[1][0], col[2][0],
			col[0][1], col[1][1], col[2][1],
			col[0][2], col[1][2], col[2][2]
		};

		T inv[9];

		inv[0] =  m[4] * m[8] - m[5] * m[7];
		inv[1] = -m[1] * m[8] + m[2] * m[7];
		inv[2] =  m[1] * m[5] - m[2] * m[4];

		inv[3] = -m[3] * m[8] + m[5] * m[6];
		inv[4] =  m[0] * m[8] - m[2] * m[6];
		inv[5] = -m[0] * m[5] + m[2] * m[3];

		inv[6] =  m[3] * m[7] - m[4] * m[6];
		inv[7] = -m[0] * m[7] + m[1] * m[6];
		inv[8] =  m[0] * m[4] - m[1] * m[3];

		const T det = m[0] * inv[0] + m[1] * inv[3] + m[2] * inv[6];
		const T inv_det = T(1) / det;

		return {
			col_type(inv[0] * inv_det, inv[1] * inv_det, inv[2] * inv_det),
			col_type(inv[3] * inv_det, inv[4] * inv_det, inv[5] * inv_det),
			col_type(inv[6] * inv_det, inv[7] * inv_det, inv[8] * inv_det)
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR const col_type& operator[](size_t idx) const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const col_type*>(&col[idx]);
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type& operator[](size_t idx) VERA_NOEXCEPT
	{
		return *reinterpret_cast<col_type*>(&col[idx]);
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+() const VERA_NOEXCEPT
	{
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-() const VERA_NOEXCEPT
	{
		return {
			-col[0],
			-col[1],
			-col[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			col[0] + rhs.col[0],
			col[1] + rhs.col[1],
			col[2] + rhs.col[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			col[0] - rhs.col[0],
			col[1] - rhs.col[1],
			col[2] - rhs.col[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x3_type operator*(const mat2x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			col[0] * rhs[0][0] + col[1] * rhs[0][1] + col[2] * rhs[0][2],
			col[0] * rhs[1][0] + col[1] * rhs[1][1] + col[2] * rhs[1][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type operator*(const mat3x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			col[0] * rhs[0][0] + col[1] * rhs[0][1] + col[2] * rhs[0][2],
			col[0] * rhs[1][0] + col[1] * rhs[1][1] + col[2] * rhs[1][2],
			col[0] * rhs[2][0] + col[1] * rhs[2][1] + col[2] * rhs[2][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x3_type operator*(const mat4x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			col[0] * rhs[0][0] + col[1] * rhs[0][1] + col[2] * rhs[0][2],
			col[0] * rhs[1][0] + col[1] * rhs[1][1] + col[2] * rhs[1][2],
			col[0] * rhs[2][0] + col[1] * rhs[2][1] + col[2] * rhs[2][2],
			col[0] * rhs[3][0] + col[1] * rhs[3][1] + col[2] * rhs[3][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type operator*(const col_type& rhs) const VERA_NOEXCEPT
	{
		return {
			col[0][0] * rhs[0] + col[1][0] * rhs[1] + col[2][0] * rhs[2],
			col[0][1] * rhs[0] + col[1][1] * rhs[1] + col[2][1] * rhs[2],
			col[0][2] * rhs[0] + col[1][2] * rhs[1] + col[2][2] * rhs[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return {
			col[0] * rhs,
			col[1] * rhs,
			col[2] * rhs
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs * rhs.col[0],
			lhs * rhs.col[1],
			lhs * rhs.col[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return {
			col[0] / rhs,
			col[1] / rhs,
			col[2] / rhs
		};
	}

	VERA_CONSTEXPR matrix_base& operator+=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		col[0] += rhs.col[0];
		col[1] += rhs.col[1];
		col[2] += rhs.col[2];
		
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator-=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		col[0] -= rhs.col[0];
		col[1] -= rhs.col[1];
		col[2] -= rhs.col[2];
		
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator*=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		return *this = *this * rhs;
	}

	VERA_CONSTEXPR matrix_base& operator*=(T rhs) VERA_NOEXCEPT
	{
		col[0] *= rhs;
		col[1] *= rhs;
		col[2] *= rhs;
	
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		col[0] /= rhs;
		col[1] /= rhs;
		col[2] /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return
			col[0] == rhs.col[0] &&
			col[1] == rhs.col[1] &&
			col[2] == rhs.col[2];
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	col_type col[col_size];
};

#endif

VERA_NAMESPACE_END
