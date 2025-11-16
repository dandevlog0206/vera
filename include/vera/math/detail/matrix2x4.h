#pragma once

#include "matrix_base.h"

VERA_NAMESPACE_BEGIN

template <class T, MathQualifier Q>
class matrix_base<2, 4, row_major, T, Q>
{
public:
	static VERA_CONSTEXPR MathDimType     row_size    = 2;
	static VERA_CONSTEXPR MathDimType     col_size    = 4;
	static VERA_CONSTEXPR MathMatrixOrder major_order = row_major;

	using row_type    = vector_base<4, T, Q>;
	using col_type    = vector_base<2, T, Q>;
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
	using this_type   = matrix_base<2, 4, row_major, T, Q>;

	static VERA_CONSTEXPR mat2x4_type identity() VERA_NOEXCEPT
	{
		return { static_cast<T>(1) };
	}

	static VERA_CONSTEXPR mat2x4_type zero() VERA_NOEXCEPT
	{
		return { static_cast<T>(0) };
	}

	VERA_CONSTEXPR matrix_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR matrix_base(T diagonal) :
		rows{
			row_type(diagonal, 0, 0, 0),
			row_type(0, diagonal, 0, 0) } {}

	VERA_CONSTEXPR matrix_base(
		T m00, T m01, T m02, T m03,
		T m10, T m11, T m12, T m13
	) VERA_NOEXCEPT :
		rows{
			row_type(m00, m01, m02, m03),
			row_type(m10, m11, m12, m13) } {}

	VERA_CONSTEXPR matrix_base(
		const row_type& row0,
		const row_type& row1
	) VERA_NOEXCEPT :
		rows{
			row0,
			row1 } {}

	VERA_CONSTEXPR matrix_base(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR matrix_base& operator=(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR T trace() const VERA_NOEXCEPT
	{
		return rows[0][0] + rows[1][1];
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x2_type transpose() const VERA_NOEXCEPT
	{
		return {
			rows[0][0], rows[1][0],
			rows[0][1], rows[1][1],
			rows[0][2], rows[1][2],
			rows[0][3], rows[1][3]
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
			-rows[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] + rhs.rows[0],
			rows[1] + rhs.rows[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] - rhs.rows[0],
			rows[1] - rhs.rows[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x2_type operator*(const mat4x2_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x3_type operator*(const mat4x3_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x4_type operator*(const mat4x4_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type operator*(const row_type& rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0][0] * rhs[0] + rows[0][1] * rhs[1] + rows[0][2] * rhs[2] + rows[0][3] * rhs[3],
			rows[1][0] * rhs[0] + rows[1][1] * rhs[1] + rows[1][2] * rhs[2] + rows[1][3] * rhs[3]
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR row_type operator*(const col_type& lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return
			lhs[0] * rhs.rows[0] +
			lhs[1] * rhs.rows[1];
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] * rhs,
			rows[1] * rhs
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs * rhs.rows[0],
			lhs * rhs.rows[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return {
			rows[0] / rhs,
			rows[1] / rhs
		};
	}

	VERA_CONSTEXPR matrix_base& operator+=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		rows[0] += rhs.rows[0];
		rows[1] += rhs.rows[1];

		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator-=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		rows[0] -= rhs.rows[0];
		rows[1] -= rhs.rows[1];

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

		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		rows[0] /= rhs;
		rows[1] /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return
			rows[0] == rhs.rows[0] &&
			rows[1] == rhs.rows[1];
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	row_type rows[row_size];
};

template <class T, MathQualifier Q>
class matrix_base<2, 4, col_major, T, Q>
{
public:
	static VERA_CONSTEXPR MathDimType     col_size    = 2;
	static VERA_CONSTEXPR MathDimType     row_size    = 4;
	static VERA_CONSTEXPR MathMatrixOrder major_order = col_major;

	using col_type    = vector_base<4, T, Q>;
	using row_type    = vector_base<2, T, Q>;
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
	using this_type   = matrix_base<2, 4, col_major, T, Q>;

	static VERA_CONSTEXPR mat2x4_type identity() VERA_NOEXCEPT
	{
		return { static_cast<T>(1) };
	}

	static VERA_CONSTEXPR mat2x4_type zero() VERA_NOEXCEPT
	{
		return { static_cast<T>(0) };
	}

	VERA_CONSTEXPR matrix_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR matrix_base(T diagonal) :
		cols{
			col_type(diagonal, 0, 0, 0),
			col_type(0, diagonal, 0, 0) } {}

	VERA_CONSTEXPR matrix_base(
		T m00, T m10,
		T m01, T m11,
		T m02, T m12,
		T m03, T m13
	) VERA_NOEXCEPT :
		cols{
			col_type(m00, m01, m02, m03),
			col_type(m10, m11, m12, m13) } {}

	VERA_CONSTEXPR matrix_base(
		const col_type& col0,
		const col_type& col1
	) VERA_NOEXCEPT :
		cols{
			col0,
			col1 } {}

	VERA_CONSTEXPR matrix_base(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR matrix_base& operator=(const matrix_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR T trace() const VERA_NOEXCEPT
	{
		return cols[0][0] + cols[1][1];
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x2_type transpose() const VERA_NOEXCEPT
	{
		return {
			cols[0][0], cols[0][1], cols[0][2], cols[0][3],
			cols[1][0], cols[1][1], cols[1][2], cols[1][3]
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
			-cols[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator+(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] + rhs.cols[0],
			cols[1] + rhs.cols[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator-(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] - rhs.cols[0],
			cols[1] - rhs.cols[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat2x2_type operator*(const mat2x2_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat3x2_type operator*(const mat3x2_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1],
			cols[0] * rhs[2][0] + cols[1] * rhs[2][1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR mat4x2_type operator*(const mat4x2_type& rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs[0][0] + cols[1] * rhs[0][1],
			cols[0] * rhs[1][0] + cols[1] * rhs[1][1],
			cols[0] * rhs[2][0] + cols[1] * rhs[2][1],
			cols[0] * rhs[3][0] + cols[1] * rhs[3][1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR col_type operator*(const row_type& rhs) const VERA_NOEXCEPT
	{
		return
			cols[0] * rhs[0] +
			cols[1] * rhs[1];
	}

	friend VERA_NODISCARD VERA_CONSTEXPR row_type operator*(const col_type& lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs[0] * rhs.cols[0][0] + lhs[1] * rhs.cols[0][1] + lhs[2] * rhs.cols[0][2] + lhs[3] * rhs.cols[0][3],
			lhs[0] * rhs.cols[1][0] + lhs[1] * rhs.cols[1][1] + lhs[2] * rhs.cols[1][2] + lhs[3] * rhs.cols[1][3]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] * rhs,
			cols[1] * rhs
		};
	}

	friend VERA_NODISCARD VERA_CONSTEXPR matrix_base operator*(T lhs, const matrix_base& rhs) VERA_NOEXCEPT
	{
		return {
			lhs * rhs.cols[0],
			lhs * rhs.cols[1]
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR matrix_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return {
			cols[0] / rhs,
			cols[1] / rhs
		};
	}

	VERA_CONSTEXPR matrix_base& operator+=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		cols[0] += rhs.cols[0];
		cols[1] += rhs.cols[1];

		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator-=(const matrix_base& rhs) VERA_NOEXCEPT
	{
		cols[0] -= rhs.cols[0];
		cols[1] -= rhs.cols[1];

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

		return *this;
	}

	VERA_CONSTEXPR matrix_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		cols[0] /= rhs;
		cols[1] /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return
			cols[0] == rhs.cols[0] &&
			cols[1] == rhs.cols[1];
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const matrix_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	col_type cols[col_size];
};

VERA_NAMESPACE_END
