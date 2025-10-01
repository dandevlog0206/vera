#pragma once

#include "vector_base.h"

VERA_NAMESPACE_BEGIN

template <class T, MathQualifier Q>
class vector_base<2, T, Q>
{
public:
	using vec2_type = vector_base<2, T, Q>;
	using vec3_type = vector_base<3, T, Q>;
	using vec4_type = vector_base<4, T, Q>;
	using this_type = vector_base<2, T, Q>;
	using elem_type = T;

	VERA_CONSTEXPR vector_base() VERA_NOEXCEPT = default;
	
	VERA_CONSTEXPR vector_base(T x, T y) VERA_NOEXCEPT  :
		x(x), y(y) {}

	VERA_CONSTEXPR vector_base(const vector_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR vector_base& operator=(const vector_base& rhs) VERA_NOEXCEPT = default;

	VERA_NODISCARD VERA_CONSTEXPR const T& operator[](size_t index) const VERA_NOEXCEPT
	{
		return reinterpret_cast<const T*>(this)[index];
	}

	VERA_NODISCARD VERA_CONSTEXPR T& operator[](size_t index) VERA_NOEXCEPT
	{
		return reinterpret_cast<T*>(this)[index];
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator+() const VERA_NOEXCEPT
	{
		return { x, y };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator-() const VERA_NOEXCEPT
	{
		return { -x, -y};
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator+(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x + rhs.x, y + rhs.y };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator-(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x - rhs.x, y - rhs.y };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x * rhs.x, y * rhs.y };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return { x * rhs, y * rhs };
	}

	friend VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(T lhs, const vector_base& rhs) VERA_NOEXCEPT
	{
		return { lhs * rhs.x, lhs * rhs.y };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator/(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x / rhs.x, y / rhs.y };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return { x / rhs, y / rhs };
	}

	VERA_CONSTEXPR vector_base& operator+=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator-=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x -= rhs.x;
		y -= rhs.y;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator*=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x *= rhs.x;
		y *= rhs.y;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator*=(T rhs) VERA_NOEXCEPT
	{
		x *= rhs;
		y *= rhs;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator/=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x /= rhs.x;
		y /= rhs.y;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		x /= rhs;
		y /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return x == rhs.x && y == rhs.y;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	T x;
	T y;
};

VERA_NAMESPACE_END
