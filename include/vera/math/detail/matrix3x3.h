#pragma once

#include "matrix_base.h"

VERA_NAMESPACE_BEGIN

template <class T, MathQualifier Q>
class matrix_base<3, 3, row_major, T, Q>
{
public:
	static VERA_CONSTEXPR MathDimType     row_size    = 3;
	static VERA_CONSTEXPR MathDimType     col_size    = 3;
	static VERA_CONSTEXPR MathMatrixOrder major_order = row_major;

	using row_type    = vector_base<3, T, Q>;
	using col_type    = vector_base<3, T, Q>;
	using vec2_type   = vector_base<2, T, Q>;
	using vec3_type   = vector_base<3, T, Q>;
	using vec4_type   = vector_base<4, T, Q>;
	using mat2x2_type = matrix_base<2, 2, row_major, T, Q>;
	using mat2x3_type = matrix_base<2, 3, row_major, T, Q>;
	using mat2x4_type = matrix_base<2, 4, row_major, T, Q>;
	using mat3x2_type = matrix_base<3, 2, row_major, T, Q>;
	using mat3x3_type = matrix_base<3, 3, row_major, T, Q>;
	using mat3x4_type = matrix_base<3, 4, row_major, T, Q>;
	using mat4x2_type = matrix_base<4, 2, row_major, T, Q>;
	using mat4x3_type = matrix_base<4, 3, row_major, T, Q>;
	using mat4x4_type = matrix_base<4, 4, row_major, T, Q>;
	using this_type   = matrix_base<3, 3, row_major, T, Q>;

	static VERA_CONSTEXPR mat3x3_type identity() VERA_NOEXCEPT
	{
		return { static_cast<T>(1) };
	}

	static VERA_CONSTEXPR mat3x3_type zero() VERA_NOEXCEPT
	{
		return { static_cast<T>(0) };
	}

	VERA_CONSTEXPR matrix_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR matrix_base(T diagonal) :
		rows{
			row_type(diagonal, 0, 0),
			row_type(0, diagonal, 0),
			row_type(0, 0, diagonal) } {}

	VERA_CONSTEXPR matrix_base(
		T m00, T m01, T m02,
		T m10, T m11, T m12,
		T m20, T m21, T m22
	) VERA_NOEXCEPT :
		rows{
			row_type(m00, m01, m02),
			row_type(m10, m11, m12),
			row_type(m20, m21, m22) } {}

	VERA_CONSTEXPR matrix_base(
		const row_type& row0,
		const row_type& row1,
		const row_type& row2
	) VERA_NOEXCEPT :
		rows{
			row0,
			row1,
			row2 } {}

	VERA_CONSTEXPR matrix_base(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR matrix_base& operator=(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR T trace() const VERA_NOEXCEPT
	{
		return rows[0][0] + rows[1][1] + rows[2][2];
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type transpose() const VERA_NOEXCEPT
	{
		return {
			rows[0][0], rows[1][0], rows[2][0],
			rows[0][1], rows[1][1], rows[2][1],
			rows[0][2], rows[1][2], rows[2][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR T det() const VERA_NOEXCEPT
	{
		return
			rows[0][0] * (rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1]) -
			rows[0][1] * (rows[1][0] * rows[2][2] - rows[1][2] * rows[2][0]) +
			rows[0][2] * (rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0]);
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type invert() const VERA_NOEXCEPT
	{
		T inv0 =  rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1];
		T inv1 = -rows[0][1] * rows[2][2] + rows[0][2] * rows[2][1];
		T inv2 =  rows[0][1] * rows[1][2] - rows[0][2] * rows[1][1];

		T inv3 = -rows[1][0] * rows[2][2] + rows[1][2] * rows[2][0];
		T inv4 =  rows[0][0] * rows[2][2] - rows[0][2] * rows[2][0];
		T inv5 = -rows[0][0] * rows[1][2] + rows[0][2] * rows[1][0];

		T inv6 =  rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0];
		T inv7 = -rows[0][0] * rows[2][1] + rows[0][1] * rows[2][0];
		T inv8 =  rows[0][0] * rows[1][1] - rows[0][1] * rows[1][0];

		const T det = rows[0][0] * inv0 + rows[0][1] * inv3 + rows[0][2] * inv6;
		const T inv_det = T(1) / det;

		return {
			row_type(inv0 * inv_det, inv1 * inv_det, inv2 * inv_det),
			row_type(inv3 * inv_det, inv4 * inv_det, inv5 * inv_det),
			row_type(inv6 * inv_det, inv7 * inv_det, inv8 * inv_det)
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR const row_type& operator[](size_t idx) const VERA_NOEXCEPT
	{
		return rows[idx];
	}

	VERA_NODISCARD VERA_CONSTEXPR row_type& operator[](size_t idx) VERA_NOEXCEPT
	{
		return rows[idx];
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+() const VERA_NOEXCEPT
	{
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-() const VERA_NOEXCEPT
	{
		return {
			-rows[0],
			-rows[1],
			-rows[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] + rhs.rows[0],
			rows[1] + rhs.rows[1],
			rows[2] + rhs.rows[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] - rhs.rows[0],
			rows[1] - rhs.rows[1],
			rows[2] - rhs.rows[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x3_type operator*(const mat3x2_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type operator*(const mat3x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2],
			rows[2][0] * rhs[0] + rows[2][1] * rhs[1] + rows[2][2] * rhs[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x3_type operator*(const mat3x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2],
			rows[2][0] * rhs[0] + rows[2][1] * rhs[1] + rows[2][2] * rhs[2],
			rows[3][0] * rhs[0] + rows[3][1] * rhs[1] + rows[3][2] * rhs[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type operator*(const row_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2],
			rows[2][0] * rhs[0] + rows[2][1] * rhs[1] + rows[2][2] * rhs[2]
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR row_type operator*(const col_type& lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return
			lhs[0] * rhs.rows[0] +
			lhs[1] * rhs.rows[1] +
			lhs[2] * rhs.rows[2];
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] * rhs,
			rows[1] * rhs,
			rows[2] * rhs
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs * rhs.rows[0],
			lhs * rhs.rows[1],
			lhs * rhs.rows[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] / rhs,
			rows[1] / rhs,
			rows[2] / rhs
		};
	}

	VERA_CONSTEXPR matrix_base& operator+=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		rows[0] += rhs.rows[0];
		rows[1] += rhs.rows[1];
		rows[2] += rhs.rows[2];

		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator-=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		rows[0] -= rhs.rows[0];
		rows[1] -= rhs.rows[1];
		rows[2] -= rhs.rows[2];

		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator*=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		return *this = *this * rhs;
	}

	VERA_CONSTEXPR matrix_base& operator*=(T rhs) VERA_NOEXCEPT
	{
		rows[0] *= rhs;
		rows[1] *= rhs;
		rows[2] *= rhs;

		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		rows[0] /= rhs;
		rows[1] /= rhs;
		rows[2] /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return
			rows[0] == rhs.rows[0] &&
			rows[1] == rhs.rows[1] &&
			rows[2] == rhs.rows[2];
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	row_type rows[row_size];
};

template <class T, MathQualifier Q>
class matrix_base<3, 3, row_major, T, Q>
{
public:
	static VERA_CONSTEXPR MathDimType     col_size    = 3;
	static VERA_CONSTEXPR MathDimType     row_size    = 3;
	static VERA_CONSTEXPR MathMatrixOrder major_order = col_major;

	using col_type    = vector_base<3, T, Q>;
	using row_type    = vector_base<3, T, Q>;
	using vec2_type   = vector_base<2, T, Q>;
	using vec3_type   = vector_base<3, T, Q>;
	using vec4_type   = vector_base<4, T, Q>;
	using mat2x2_type = matrix_base<2, 2, col_major, T, Q>;
	using mat2x3_type = matrix_base<2, 3, col_major, T, Q>;
	using mat2x4_type = matrix_base<2, 4, col_major, T, Q>;
	using mat3x2_type = matrix_base<3, 2, col_major, T, Q>;
	using mat3x3_type = matrix_base<3, 3, col_major, T, Q>;
	using mat3x4_type = matrix_base<3, 4, col_major, T, Q>;
	using mat4x2_type = matrix_base<4, 2, col_major, T, Q>;
	using mat4x3_type = matrix_base<4, 3, col_major, T, Q>;
	using mat4x4_type = matrix_base<4, 4, col_major, T, Q>;
	using this_type   = matrix_base<3, 3, col_major, T, Q>;

	static VERA_CONSTEXPR mat3x3_type identity() VERA_NOEXCEPT
	{
		return { static_cast<T>(1) };
	}

	static VERA_CONSTEXPR mat3x3_type zero() VERA_NOEXCEPT
	{
		return { static_cast<T>(0) };
	}

	VERA_CONSTEXPR matrix_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR matrix_base(T diagonal) :
		cols{
			col_type(diagonal, 0, 0),
			col_type(0, diagonal, 0),
			col_type(0, 0, diagonal) } {}

	VERA_CONSTEXPR matrix_base(
		T m00, T m10, T m20,
		T m01, T m11, T m21,
		T m02, T m12, T m22
	) VERA_NOEXCEPT :
		cols{
			col_type(m00, m01, m02),
			col_type(m10, m11, m12),
			col_type(m20, m21, m22) } {}

	VERA_CONSTEXPR matrix_base(
		const col_type& col0,
		const col_type& col1,
		const col_type& col2
	) VERA_NOEXCEPT :
		cols{
			col0,
			col1,
			col2 } {}

	VERA_CONSTEXPR matrix_base(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR matrix_base& operator=(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR T trace() const VERA_NOEXCEPT
	{
		return cols[0][0] + cols[1][1] + cols[2][2];
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type transpose() const VERA_NOEXCEPT
	{
		return {
			cols[0][0], cols[0][1], cols[0][2],
			cols[1][0], cols[1][1], cols[1][2],
			cols[2][0], cols[2][1], cols[2][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR T det() const VERA_NOEXCEPT
	{
		return
			cols[0][0] * (cols[1][1] * cols[2][2] - cols[1][2] * cols[2][1]) -
			cols[0][1] * (cols[1][0] * cols[2][2] - cols[1][2] * cols[2][0]) +
			cols[0][2] * (cols[1][0] * cols[2][1] - cols[1][1] * cols[2][0]);
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type invert() const VERA_NOEXCEPT
	{
		const T m[9] = {
			cols[0][0], cols[1][0], cols[2][0],
			cols[0][1], cols[1][1], cols[2][1],
			cols[0][2], cols[1][2], cols[2][2]
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
		return *reinterpret_cast<const col_type*>(&cols[idx]);
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type& operator[](size_t idx) VERA_NOEXCEPT
	{
		return *reinterpret_cast<col_type*>(&cols[idx]);
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+() const VERA_NOEXCEPT
	{
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-() const VERA_NOEXCEPT
	{
		return {
			-cols[0],
			-cols[1],
			-cols[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] + rhs.cols[0],
			cols[1] + rhs.cols[1],
			cols[2] + rhs.cols[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] - rhs.cols[0],
			cols[1] - rhs.cols[1],
			cols[2] - rhs.cols[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x3_type operator*(const mat2x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1] + cols[2] * rhs[0][2],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1] + cols[2] * rhs[1][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x3_type operator*(const mat3x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1] + cols[2] * rhs[0][2],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1] + cols[2] * rhs[1][2],
			cols[0] * rhs[2][0] + cols[1] * rhs[2][1] + cols[2] * rhs[2][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x3_type operator*(const mat4x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1] + cols[2] * rhs[0][2],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1] + cols[2] * rhs[1][2],
			cols[0] * rhs[2][0] + cols[1] * rhs[2][1] + cols[2] * rhs[2][2],
			cols[0] * rhs[3][0] + cols[1] * rhs[3][1] + cols[2] * rhs[3][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type operator*(const row_type& rhs) const VERA_NOEXCEPT
	{
		return
			cols[0] * rhs[0] +
			cols[1] * rhs[1] +
			cols[2] * rhs[2];
	}

	friend VERA_NODISCARD VERA_CONSTEXPR row_type operator*(const col_type& lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs[0] * rhs.cols[0][0] + lhs[1] * rhs.cols[0][1] + lhs[2] * rhs.cols[0][2],
			lhs[0] * rhs.cols[1][0] + lhs[1] * rhs.cols[1][1] + lhs[2] * rhs.cols[1][2],
			lhs[0] * rhs.cols[2][0] + lhs[1] * rhs.cols[2][1] + lhs[2] * rhs.cols[2][2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs,
			cols[1] * rhs,
			cols[2] * rhs
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs * rhs.cols[0],
			lhs * rhs.cols[1],
			lhs * rhs.cols[2]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] / rhs,
			cols[1] / rhs,
			cols[2] / rhs
		};
	}

	VERA_CONSTEXPR matrix_base& operator+=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		cols[0] += rhs.cols[0];
		cols[1] += rhs.cols[1];
		cols[2] += rhs.cols[2];
		
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator-=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		cols[0] -= rhs.cols[0];
		cols[1] -= rhs.cols[1];
		cols[2] -= rhs.cols[2];
		
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator*=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		return *this = *this * rhs;
	}

	VERA_CONSTEXPR matrix_base& operator*=(T rhs) VERA_NOEXCEPT
	{
		cols[0] *= rhs;
		cols[1] *= rhs;
		cols[2] *= rhs;
	
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		cols[0] /= rhs;
		cols[1] /= rhs;
		cols[2] /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return
			cols[0] == rhs.cols[0] &&
			cols[1] == rhs.cols[1] &&
			cols[2] == rhs.cols[2];
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	col_type cols[col_size];
};

VERA_NAMESPACE_END
