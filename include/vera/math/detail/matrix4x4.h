#pragma once

#include "matrix_base.h"

VERA_NAMESPACE_BEGIN

template <class T, MathQualifier Q>
class matrix_base<4, 4, row_major, T, Q>
{
public:
	static VERA_CONSTEXPR MathDimType     row_size    = 4;
	static VERA_CONSTEXPR MathDimType     col_size    = 4;
	static VERA_CONSTEXPR MathMatrixOrder major_order = row_major;

	using row_type    = vector_base<4, T, Q>;
	using col_type    = vector_base<4, T, Q>;
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
	using this_type   = matrix_base<4, 4, row_major, T, Q>;

	static VERA_CONSTEXPR mat4x4_type identity() VERA_NOEXCEPT
	{
		return { static_cast<T>(1) };
	}

	static VERA_CONSTEXPR mat4x4_type zero() VERA_NOEXCEPT
	{
		return { static_cast<T>(0) };
	}

	VERA_CONSTEXPR matrix_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR matrix_base(T diagonal) :
		rows{
			row_type(diagonal, 0, 0, 0),
			row_type(0, diagonal, 0, 0),
			row_type(0, 0, diagonal, 0),
			row_type(0, 0, 0, diagonal) } {}

	VERA_CONSTEXPR matrix_base(
		T m00, T m01, T m02, T m03,
		T m10, T m11, T m12, T m13,
		T m20, T m21, T m22, T m23,
		T m30, T m31, T m32, T m33
	) VERA_NOEXCEPT :
		rows{
			row_type(m00, m01, m02, m03),
			row_type(m10, m11, m12, m13),
			row_type(m20, m21, m22, m23),
			row_type(m30, m31, m32, m33) } {}

	VERA_CONSTEXPR matrix_base(
		const row_type& row0,
		const row_type& row1,
		const row_type& row2,
		const row_type& row3
	) VERA_NOEXCEPT :
		rows{
			row0,
			row1,
			row2,
			row3 } {}

	VERA_CONSTEXPR matrix_base(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR matrix_base& operator=(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR T trace() const VERA_NOEXCEPT
	{
		return rows[0][0] + rows[1][1] + rows[2][2] + rows[3][3];
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x4_type transpose() const VERA_NOEXCEPT
	{
		return {
			rows[0][0], rows[1][0], rows[2][0], rows[3][0],
			rows[0][1], rows[1][1], rows[2][1], rows[3][1],
			rows[0][2], rows[1][2], rows[2][2], rows[3][2],
			rows[0][3], rows[1][3], rows[2][3], rows[3][3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR T det() const VERA_NOEXCEPT
	{
		const T det0 = rows[0][0] * (
			+rows[1][1] * (rows[2][2] * rows[3][3] - rows[2][3] * rows[3][2])
			-rows[1][2] * (rows[2][1] * rows[3][3] - rows[2][3] * rows[3][1])
			+rows[1][3] * (rows[2][1] * rows[3][2] - rows[2][2] * rows[3][1]));

		const T det1 = rows[0][1] * (
			+rows[1][0] * (rows[2][2] * rows[3][3] - rows[2][3] * rows[3][2])
			-rows[1][2] * (rows[2][0] * rows[3][3] - rows[2][3] * rows[3][0])
			+rows[1][3] * (rows[2][0] * rows[3][2] - rows[2][2] * rows[3][0]));

		const T det2 = rows[0][2] * (
			+rows[1][0] * (rows[2][1] * rows[3][3] - rows[2][3] * rows[3][1])
			-rows[1][1] * (rows[2][0] * rows[3][3] - rows[2][3] * rows[3][0])
			+rows[1][3] * (rows[2][0] * rows[3][1] - rows[2][1] * rows[3][0]));

		const T det3 = rows[0][3] * (
			+rows[1][0] * (rows[2][1] * rows[3][2] - rows[2][2] * rows[3][1])
			-rows[1][1] * (rows[2][0] * rows[3][2] - rows[2][2] * rows[3][0])
			+rows[1][2] * (rows[2][0] * rows[3][1] - rows[2][1] * rows[3][0]));

		return det0 - det1 + det2 - det3;
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x4_type inv() const VERA_NOEXCEPT
	{
		const T m[16] = {
			rows[0][0], rows[1][0], rows[2][0], rows[3][0],
			rows[0][1], rows[1][1], rows[2][1], rows[3][1],
			rows[0][2], rows[1][2], rows[2][2], rows[3][2],
			rows[0][3], rows[1][3], rows[2][3], rows[3][3]
		};

		T inv[16];

		inv[0]  = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
		inv[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
		inv[8]  = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
		inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

		inv[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
		inv[5]  = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
		inv[9]  = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
		inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

		inv[2]  = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
		inv[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
		inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
		inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

		inv[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
		inv[7]  = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
		inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
		inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

		const T det     = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
		const T inv_det = T(1) / det;

		return {
			row_type(inv[0] * inv_det, inv[1] * inv_det, inv[2] * inv_det, inv[3] * inv_det),
			row_type(inv[4] * inv_det, inv[5] * inv_det, inv[6] * inv_det, inv[7] * inv_det),
			row_type(inv[8] * inv_det, inv[9] * inv_det, inv[10] * inv_det, inv[11] * inv_det),
			row_type(inv[12] * inv_det, inv[13] * inv_det, inv[14] * inv_det, inv[15] * inv_det)
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
			-rows[2],
			-rows[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] + rhs.rows[0],
			rows[1] + rhs.rows[1],
			rows[2] + rhs.rows[2],
			rows[3] + rhs.rows[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] - rhs.rows[0],
			rows[1] - rhs.rows[1],
			rows[2] - rhs.rows[2],
			rows[3] - rhs.rows[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x4_type operator*(const mat2x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3],
			rows[2][0] * rhs[0] + rows[2][1] * rhs[1] + rows[2][2] * rhs[2] + rows[2][3] * rhs[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x4_type operator*(const mat3x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3],
			rows[2][0] * rhs[0] + rows[2][1] * rhs[1] + rows[2][2] * rhs[2] + rows[2][3] * rhs[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x4_type operator*(const mat4x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3],
			rows[2][0] * rhs[0] + rows[2][1] * rhs[1] + rows[2][2] * rhs[2] + rows[2][3] * rhs[3],
			rows[3][0] * rhs[0] + rows[3][1] * rhs[1] + rows[3][2] * rhs[2] + rows[3][3] * rhs[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type operator*(const row_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3],
			rows[2][0] * rhs[0] + rows[2][1] * rhs[1] + rows[2][2] * rhs[2] + rows[2][3] * rhs[3],
			rows[3][0] * rhs[0] + rows[3][1] * rhs[1] + rows[3][2] * rhs[2] + rows[3][3] * rhs[3]
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR row_type operator*(const col_type& lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return
			lhs[0] * rhs.rows[0] +
			lhs[1] * rhs.rows[1] +
			lhs[2] * rhs.rows[2] +
			lhs[3] * rhs.rows[3];
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] * rhs,
			rows[1] * rhs,
			rows[2] * rhs,
			rows[3] * rhs
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs * rhs.rows[0],
			lhs * rhs.rows[1],
			lhs * rhs.rows[2],
			lhs * rhs.rows[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] / rhs,
			rows[1] / rhs,
			rows[2] / rhs,
			rows[3] / rhs
		};
	}

	VERA_CONSTEXPR matrix_base& operator+=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		rows[0] += rhs.rows[0];
		rows[1] += rhs.rows[1];
		rows[2] += rhs.rows[2];
		rows[3] += rhs.rows[3];
		
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator-=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		rows[0] -= rhs.rows[0];
		rows[1] -= rhs.rows[1];
		rows[2] -= rhs.rows[2];
		rows[3] -= rhs.rows[3];
		
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
		rows[3] *= rhs;
	
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		rows[0] /= rhs;
		rows[1] /= rhs;
		rows[2] /= rhs;
		rows[3] /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return 
			rows[0] == rhs.rows[0] &&
			rows[1] == rhs.rows[1] &&
			rows[2] == rhs.rows[2] &&
			rows[3] == rhs.rows[3];
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	row_type rows[row_size];
};

template <class T, MathQualifier Q>
class matrix_base<4, 4, col_major, T, Q>
{
public:
	static VERA_CONSTEXPR MathDimType     col_size    = 4;
	static VERA_CONSTEXPR MathDimType     row_size    = 4;
	static VERA_CONSTEXPR MathMatrixOrder major_order = col_major;

	using col_type    = vector_base<4, T, Q>;
	using row_type    = vector_base<4, T, Q>;
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
	using this_type   = matrix_base<4, 4, col_major, T, Q>;

	static VERA_CONSTEXPR mat4x4_type identity() VERA_NOEXCEPT
	{
		return { static_cast<T>(1) };
	}

	static VERA_CONSTEXPR mat4x4_type zero() VERA_NOEXCEPT
	{
		return { static_cast<T>(0) };
	}

	VERA_CONSTEXPR matrix_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR matrix_base(T diagonal) :
		cols{
			col_type(diagonal, 0, 0, 0),
			col_type(0, diagonal, 0, 0),
			col_type(0, 0, diagonal, 0),
			col_type(0, 0, 0, diagonal) } {}

	VERA_CONSTEXPR matrix_base(
		T m00, T m10, T m20, T m30,
		T m01, T m11, T m21, T m31,
		T m02, T m12, T m22, T m32,
		T m03, T m13, T m23, T m33
	) VERA_NOEXCEPT :
		cols{
			col_type(m00, m01, m02, m03),
			col_type(m10, m11, m12, m13),
			col_type(m20, m21, m22, m23),
			col_type(m30, m31, m32, m33) } {}

	VERA_CONSTEXPR matrix_base(
		const col_type& col0,
		const col_type& col1,
		const col_type& col2,
		const col_type& col3
	) VERA_NOEXCEPT :
		cols{
			col0,
			col1,
			col2,
			col3 } {}

	VERA_CONSTEXPR matrix_base(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR matrix_base& operator=(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR T trace() const VERA_NOEXCEPT
	{
		return cols[0][0] + cols[1][1] + cols[2][2] + cols[3][3];
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x4_type transpose() const VERA_NOEXCEPT
	{
		return {
			cols[0][0], cols[0][1], cols[0][2], cols[0][3],
			cols[1][0], cols[1][1], cols[1][2], cols[1][3],
			cols[2][0], cols[2][1], cols[2][2], cols[2][3],
			cols[3][0], cols[3][1], cols[3][2], cols[3][3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR T det() const VERA_NOEXCEPT
	{
		const T det0 = cols[0][0] * (
			+cols[1][1] * (cols[2][2] * cols[3][3] - cols[2][3] * cols[3][2])
			-cols[1][2] * (cols[2][1] * cols[3][3] - cols[2][3] * cols[3][1])
			+cols[1][3] * (cols[2][1] * cols[3][2] - cols[2][2] * cols[3][1]));

		const T det1 = cols[0][1] * (
			+cols[1][0] * (cols[2][2] * cols[3][3] - cols[2][3] * cols[3][2])
			-cols[1][2] * (cols[2][0] * cols[3][3] - cols[2][3] * cols[3][0])
			+cols[1][3] * (cols[2][0] * cols[3][2] - cols[2][2] * cols[3][0]));

		const T det2 = cols[0][2] * (
			+cols[1][0] * (cols[2][1] * cols[3][3] - cols[2][3] * cols[3][1])
			-cols[1][1] * (cols[2][0] * cols[3][3] - cols[2][3] * cols[3][0])
			+cols[1][3] * (cols[2][0] * cols[3][1] - cols[2][1] * cols[3][0]));

		const T det3 = cols[0][3] * (
			+cols[1][0] * (cols[2][1] * cols[3][2] - cols[2][2] * cols[3][1])
			-cols[1][1] * (cols[2][0] * cols[3][2] - cols[2][2] * cols[3][0])
			+cols[1][2] * (cols[2][0] * cols[3][1] - cols[2][1] * cols[3][0]));

		return det0 - det1 + det2 - det3;
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x4_type inv() const VERA_NOEXCEPT
	{
		const T m[16] = {
			cols[0][0], cols[1][0], cols[2][0], cols[3][0],
			cols[0][1], cols[1][1], cols[2][1], cols[3][1],
			cols[0][2], cols[1][2], cols[2][2], cols[3][2],
			cols[0][3], cols[1][3], cols[2][3], cols[3][3]
		};

		T inv[16];

		inv[0]  = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
		inv[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
		inv[8]  = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
		inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

		inv[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
		inv[5]  = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
		inv[9]  = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
		inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

		inv[2]  = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
		inv[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
		inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
		inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

		inv[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
		inv[7]  = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
		inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
		inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

		const T det     = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
		const T inv_det = T(1) / det;

		return {
			col_type(inv[0] * inv_det, inv[1] * inv_det, inv[2] * inv_det, inv[3] * inv_det),
			col_type(inv[4] * inv_det, inv[5] * inv_det, inv[6] * inv_det, inv[7] * inv_det),
			col_type(inv[8] * inv_det, inv[9] * inv_det, inv[10] * inv_det, inv[11] * inv_det),
			col_type(inv[12] * inv_det, inv[13] * inv_det, inv[14] * inv_det, inv[15] * inv_det)
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR const col_type& operator[](size_t idx) const VERA_NOEXCEPT
	{
		return cols[idx];
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type& operator[](size_t idx) VERA_NOEXCEPT
	{
		return cols[idx];
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
			-cols[2],
			-cols[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] + rhs.cols[0],
			cols[1] + rhs.cols[1],
			cols[2] + rhs.cols[2],
			cols[3] + rhs.cols[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] - rhs.cols[0],
			cols[1] - rhs.cols[1],
			cols[2] - rhs.cols[2],
			cols[3] - rhs.cols[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x4_type operator*(const mat2x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1] + cols[2] * rhs[0][2] + cols[3] * rhs[0][3],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1] + cols[2] * rhs[1][2] + cols[3] * rhs[1][3],
			cols[0] * rhs[2][0] + cols[1] * rhs[2][1] + cols[2] * rhs[2][2] + cols[3] * rhs[2][3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x4_type operator*(const mat3x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1] + cols[2] * rhs[0][2] + cols[3] * rhs[0][3],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1] + cols[2] * rhs[1][2] + cols[3] * rhs[1][3],
			cols[0] * rhs[2][0] + cols[1] * rhs[2][1] + cols[2] * rhs[2][2] + cols[3] * rhs[2][3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x4_type operator*(const mat4x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1] + cols[2] * rhs[0][2] + cols[3] * rhs[0][3],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1] + cols[2] * rhs[1][2] + cols[3] * rhs[1][3],
			cols[0] * rhs[2][0] + cols[1] * rhs[2][1] + cols[2] * rhs[2][2] + cols[3] * rhs[2][3],
			cols[0] * rhs[3][0] + cols[1] * rhs[3][1] + cols[2] * rhs[3][2] + cols[3] * rhs[3][3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type operator*(const row_type& rhs) const VERA_NOEXCEPT
	{
		return
			cols[0] * rhs[0] +
			cols[1] * rhs[1] +
			cols[2] * rhs[2] +
			cols[3] * rhs[3];
	}

	friend VERA_NODISCARD VERA_CONSTEXPR row_type operator*(const col_type& lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs[0] * rhs.cols[0][0] + lhs[1] * rhs.cols[0][1] + lhs[2] * rhs.cols[0][2] + lhs[3] * rhs.cols[0][3],
			lhs[0] * rhs.cols[1][0] + lhs[1] * rhs.cols[1][1] + lhs[2] * rhs.cols[1][2] + lhs[3] * rhs.cols[1][3],
			lhs[0] * rhs.cols[2][0] + lhs[1] * rhs.cols[2][1] + lhs[2] * rhs.cols[2][2] + lhs[3] * rhs.cols[2][3],
			lhs[0] * rhs.cols[3][0] + lhs[1] * rhs.cols[3][1] + lhs[2] * rhs.cols[3][2] + lhs[3] * rhs.cols[3][3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs,
			cols[1] * rhs,
			cols[2] * rhs,
			cols[3] * rhs
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs * rhs.cols[0],
			lhs * rhs.cols[1],
			lhs * rhs.cols[2],
			lhs * rhs.cols[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] / rhs,
			cols[1] / rhs,
			cols[2] / rhs,
			cols[3] / rhs
		};
	}

	VERA_CONSTEXPR matrix_base& operator+=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		cols[0] += rhs.cols[0];
		cols[1] += rhs.cols[1];
		cols[2] += rhs.cols[2];
		cols[3] += rhs.cols[3];
		
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator-=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		cols[0] -= rhs.cols[0];
		cols[1] -= rhs.cols[1];
		cols[2] -= rhs.cols[2];
		cols[3] -= rhs.cols[3];
		
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
		cols[3] *= rhs;
	
		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		cols[0] /= rhs;
		cols[1] /= rhs;
		cols[2] /= rhs;
		cols[3] /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return 
			cols[0] == rhs.cols[0] &&
			cols[1] == rhs.cols[1] &&
			cols[2] == rhs.cols[2] &&
			cols[3] == rhs.cols[3];
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	col_type cols[col_size];
};

VERA_NAMESPACE_END
