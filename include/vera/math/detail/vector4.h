#pragma once

#include "vector3.h"

VERA_NAMESPACE_BEGIN

template <class T, MathQualifier Q>
class vector_base<4, T, Q>
{
public:
	using vec2_type = vector_base<2, T, Q>;
	using vec3_type = vector_base<3, T, Q>;
	using vec4_type = vector_base<4, T, Q>;
	using this_type = vector_base<4, T, Q>;
	using elem_type = T;

	VERA_CONSTEXPR vector_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR vector_base(T init) VERA_NOEXCEPT :
		x(init), y(init), z(init), w(init) {}

	VERA_CONSTEXPR vector_base(T x, T y, T z, T w) VERA_NOEXCEPT :
		x(x), y(y), z(z), w(w) {}

	VERA_CONSTEXPR vector_base(const vec2_type& xy, T z, T w) VERA_NOEXCEPT :
		x(xy.x), y(xy.y), z(z), w(w) {}

	VERA_CONSTEXPR vector_base(T x, const vec2_type& yz, T w) VERA_NOEXCEPT :
		x(x), y(yz.y), z(yz.z), w(w) {}

	VERA_CONSTEXPR vector_base(T x, T y, const vec2_type& zw) VERA_NOEXCEPT :
		x(x), y(y), z(zw.z), w(zw.w) {}

	VERA_CONSTEXPR vector_base(const vec3_type& xyz, T w) VERA_NOEXCEPT :
		x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

	VERA_CONSTEXPR vector_base(T x, const vec3_type& yzw) VERA_NOEXCEPT :
		x(x), y(yzw.y), z(yzw.z), w(yzw.w) {}

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
		return { -x, -y, -z, -w };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator+(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator-(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(T rhs) const VERA_NOEXCEPT
	{
		return { x * rhs, y * rhs, z * rhs, w * rhs };
	}

	friend VERA_NODISCARD VERA_CONSTEXPR vector_base operator*(T lhs, const vector_base& rhs) VERA_NOEXCEPT
	{
		return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator/(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return { x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR vector_base operator/(T rhs) const VERA_NOEXCEPT
	{
		return { x / rhs, y / rhs, z / rhs, w / rhs };
	}

	VERA_CONSTEXPR vector_base& operator+=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator-=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator*=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		w *= rhs.w;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator*=(T rhs) VERA_NOEXCEPT
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		w *= rhs;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator/=(const vector_base& rhs) VERA_NOEXCEPT
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		w /= rhs.w;

		return *this;
	}

	VERA_CONSTEXPR vector_base& operator/=(T rhs) VERA_NOEXCEPT
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const vector_base& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	T x;
	T y;
	T z;
	T w;
};

VERA_NAMESPACE_END