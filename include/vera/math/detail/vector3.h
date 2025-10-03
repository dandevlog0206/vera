#pragma once

#include "vector2.h"

VERA_NAMESPACE_BEGIN

template <class T, MathQualifier Q>
class vector_base<3, T, Q>
{
public:
	using vec2_type = vector_base<2, T, Q>;
	using vec3_type = vector_base<3, T, Q>;
	using vec4_type = vector_base<4, T, Q>;
	using this_type = vector_base<3, T, Q>;
	using elem_type = T;

	VERA_CONSTEXPR vector_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR vector_base(T x, T y, T z) VERA_NOEXCEPT :
		x(x), y(y), z(z) {}
	
	VERA_CONSTEXPR vector_base(const vec2_type& xy, T z) VERA_NOEXCEPT :
		x(xy.x), y(xy.y), z(z) {}

	VERA_CONSTEXPR vector_base(T x, const vec2_type& yz) VERA_NOEXCEPT :
		x(x), y(yz.y), z(yz.z) {}
	
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
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator-() const VERA_NOEXCEPT
	{
		return { -x, -y, -z };
	}
	
	VERA_NODISCARD VERA_CONSTEXPR vector_base operator+(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x + rhs.x, y + rhs.y, z + rhs.z };
	}
	
	VERA_NODISCARD VERA_CONSTEXPR vector_base operator-(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}
	
	VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x * rhs.x, y * rhs.y, z * rhs.z };
	}
	
	VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return { x * rhs, y * rhs, z * rhs };
	}
	
	friend VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(T lhs, const vector_base& rhs) VERA_NOEXCEPT
	{
		return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator/(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x / rhs.x, y / rhs.y, z / rhs.z };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return { x / rhs, y / rhs, z / rhs };
	}

	VERA_CONSTEXPR vector_base& operator+=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator-=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
	
		return *this;
	}

	VERA_CONSTEXPR vector_base& operator*=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator*=(T rhs) VERA_NOEXCEPT
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator/=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	T x;
	T y;
	T z;
};

VERA_NAMESPACE_END