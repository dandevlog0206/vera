#pragma once

#include "../core/assertion.h"
#include "vector_types.h"
#include "matrix_types.h"
#include "radian.h"
#include <cmath>

VERA_NAMESPACE_BEGIN

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<2, T, Q> abs(const vector_base<2, T, Q>& v) VERA_NOEXCEPT
{
	return {
		static_cast<T>(0.0) < v.x ? v.x : -v.x,
		static_cast<T>(0.0) < v.y ? v.y : -v.y
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> abs(const vector_base<3, T, Q>& v) VERA_NOEXCEPT
{
	return {
		static_cast<T>(0.0) < v.x ? v.x : -v.x,
		static_cast<T>(0.0) < v.y ? v.y : -v.y,
		static_cast<T>(0.0) < v.z ? v.z : -v.z
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<4, T, Q> abs(const vector_base<4, T, Q>& v) VERA_NOEXCEPT
{
	return {
		static_cast<T>(0.0) < v.x ? v.x : -v.x,
		static_cast<T>(0.0) < v.y ? v.y : -v.y,
		static_cast<T>(0.0) < v.z ? v.z : -v.z,
		static_cast<T>(0.0) < v.w ? v.w : -v.w
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<2, T, Q> saturate(const vector_base<2, T, Q>& v) VERA_NOEXCEPT
{
	return {
		v.x < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.x ? static_cast<T>(1.0) : v.x),
		v.y < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.y ? static_cast<T>(1.0) : v.y)
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> saturate(const vector_base<3, T, Q>& v) VERA_NOEXCEPT
{
	return {
		v.x < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.x ? static_cast<T>(1.0) : v.x),
		v.y < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.y ? static_cast<T>(1.0) : v.y),
		v.z < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.z ? static_cast<T>(1.0) : v.z)
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<4, T, Q> saturate(const vector_base<4, T, Q>& v) VERA_NOEXCEPT
{
	return {
		v.x < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.x ? static_cast<T>(1.0) : v.x),
		v.y < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.y ? static_cast<T>(1.0) : v.y),
		v.z < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.z ? static_cast<T>(1.0) : v.z),
		v.w < static_cast<T>(0.0) ? static_cast<T>(0.0) : (static_cast<T>(1.0) < v.w ? static_cast<T>(1.0) : v.w)
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR T length(const vector_base<2, T, Q>& v) VERA_NOEXCEPT
{
	return std::sqrt(v.x * v.x + v.y * v.y);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR T length(const vector_base<3, T, Q>& v) VERA_NOEXCEPT
{
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR T length(const vector_base<4, T, Q>& v) VERA_NOEXCEPT
{
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

template <size_t Dim, class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR T distance(const vector_base<Dim, T, Q>& lhs, const vector_base<Dim, T, Q>& rhs) VERA_NOEXCEPT
{
	return length(rhs - lhs);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR T dot(const vector_base<2, T, Q>& lhs, const vector_base<2, T, Q>& rhs) VERA_NOEXCEPT
{
	return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR T dot(const vector_base<3, T, Q>& lhs, const vector_base<3, T, Q>& rhs) VERA_NOEXCEPT
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR T dot(const vector_base<4, T, Q>& lhs, const vector_base<4, T, Q>& rhs) VERA_NOEXCEPT
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR float cross(const vector_base<2, T, Q>& lhs, const vector_base<2, T, Q>& rhs) VERA_NOEXCEPT
{
	return lhs.x * rhs.y - lhs.y * rhs.x;
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> cross(const vector_base<3, T, Q>& lhs, const vector_base<3, T, Q>& rhs) VERA_NOEXCEPT
{
	return {
		lhs.y * rhs.z - rhs.y * lhs.z,
		lhs.z * rhs.x - rhs.z * lhs.x,
		lhs.x * rhs.y - rhs.x * lhs.y };
}

template <size_t Dim, class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<Dim, T, Q> normalize(const vector_base<Dim, T, Q>& v) VERA_NOEXCEPT
{
	return v / length(v);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR float shoelace(const vector_base<2, T, Q>& lhs, const vector_base<2, T, Q>& rhs) VERA_NOEXCEPT
{
	return (rhs.x - lhs.x) * (lhs.y + rhs.y);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<2, T, Q> lerp(
	const vector_base<2, T, Q>& p0,
	const vector_base<2, T, Q>& p1,
	T                           t
) VERA_NOEXCEPT {
	return {
		p0.x + (p1.x - p0.x) * t,
		p0.y + (p1.y - p0.y) * t
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> lerp(
	const vector_base<3, T, Q>& p0,
	const vector_base<3, T, Q>& p1,
	T                           t
) VERA_NOEXCEPT {
	return {
		p0.x + (p1.x - p0.x) * t,
		p0.y + (p1.y - p0.y) * t,
		p0.z + (p1.z - p0.z) * t
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<4, T, Q> lerp(
	const vector_base<4, T, Q>& p0,
	const vector_base<4, T, Q>& p1,
	T                           t
) VERA_NOEXCEPT {
	return {
		p0.x + (p1.x - p0.x) * t,
		p0.y + (p1.y - p0.y) * t,
		p0.z + (p1.z - p0.z) * t,
		p0.w + (p1.w - p0.w) * t
	};
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<2, T, Q> quadratic(
	const vector_base<2, T, Q>& p0,
	const vector_base<2, T, Q>& p1,
	const vector_base<2, T, Q>& p2,
	T                           t
) VERA_NOEXCEPT {
	return lerp(lerp(p0, p1, t), lerp(p1, p2, t), t);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> quadratic(
	const vector_base<3, T, Q>& p0,
	const vector_base<3, T, Q>& p1,
	const vector_base<3, T, Q>& p2,
	T                           t
) VERA_NOEXCEPT {
	return lerp(lerp(p0, p1, t), lerp(p1, p2, t), t);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<2, T, Q> cubic(
	const vector_base<2, T, Q>& p0,
	const vector_base<2, T, Q>& p1,
	const vector_base<2, T, Q>& p2,
	const vector_base<2, T, Q>& p3,
	T                           t
) VERA_NOEXCEPT {
	return lerp(quadratic(p0, p1, p2, t), quadratic(p0, p1, p2, t), t);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> cubic(
	const vector_base<3, T, Q>& p0,
	const vector_base<3, T, Q>& p1,
	const vector_base<3, T, Q>& p2,
	const vector_base<3, T, Q>& p3,
	T                           t
) VERA_NOEXCEPT {
	return lerp(quadratic(p0, p1, p2, t), quadratic(p0, p1, p2, t), t);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<2, T, Q> reflect(
	const vector_base<2, T, Q>& incident,
	const vector_base<2, T, Q>& normal
) VERA_NOEXCEPT {
	const T dot_value = dot(normal, incident);

	return incident - normal * dot_value * static_cast<T>(2);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> reflect(
	const vector_base<3, T, Q>& incident,
	const vector_base<3, T, Q>& normal
) VERA_NOEXCEPT {
	const T dot_value = dot(normal, incident);

	return incident - normal * dot_value * static_cast<T>(2);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<2, T, Q> refract(
	const vector_base<2, T, Q>& incident,
	const vector_base<2, T, Q>& normal,
	T                           eta
) VERA_NOEXCEPT {
	const T dot_value = dot(normal, incident);
	const T k         = static_cast<T>(1) - eta * eta * (static_cast<T>(1) - dot_value * dot_value);

	return
		k >= static_cast<T>(0) ?
		(eta * incident - (eta * dot_value + std::sqrt(k)) * normal) :
		vector_base<3, T, Q>(0);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T, Q> refract(
	const vector_base<3, T, Q>& incident,
	const vector_base<3, T, Q>& normal,
	T                           eta
) VERA_NOEXCEPT {
	const T dot_value = dot(normal, incident);
	const T k         = static_cast<T>(1) - eta * eta * (static_cast<T>(1) - dot_value * dot_value);

	return
		k >= static_cast<T>(0) ?
		(eta * incident - (eta * dot_value + std::sqrt(k)) * normal) :
		vector_base<3, T, Q>(0);
}

template <class T, MathQualifier Q>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, default_major, T, Q> lookAt(
	const vector_base<3, T, Q>& eye,
	const vector_base<3, T, Q>& center,
	const vector_base<3, T, Q>& up
) {
	const vector_base<3, T, Q> f(normalize(center - eye));
	const vector_base<3, T, Q> s(normalize(cross(up, f)));
	const vector_base<3, T, Q> u(cross(f, s));

	matrix_base<4, 4, default_major, T, Q> result(static_cast<T>(1));

	if constexpr (default_major == row_major) {
		result[0][0] = s.x;
		result[0][1] = s.y;
		result[0][2] = s.z;
		result[1][0] = u.x;
		result[1][1] = u.y;
		result[1][2] = u.z;
		result[2][0] = -f.x;
		result[2][1] = -f.y;
		result[2][2] = -f.z;
		result[0][3] = -dot(s, eye);
		result[1][3] = -dot(u, eye);
		result[2][3] = dot(f, eye);
	} else {
		result[0][0] = s.x;
		result[1][0] = s.y;
		result[2][0] = s.z;
		result[0][1] = u.x;
		result[1][1] = u.y;
		result[2][1] = u.z;
		result[0][2] = f.x;
		result[1][2] = f.y;
		result[2][2] = f.z;
		result[3][0] = -dot(s, eye);
		result[3][1] = -dot(u, eye);
		result[3][2] = -dot(f, eye);
	}

	return result;
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, default_major, T, packed_highp> ortho(
	T left,
	T right,
	T bottom,
	T top
) {
	matrix_base<4, 4, default_major, T, packed_highp> result(static_cast<T>(1));

	if constexpr (default_major == row_major) {
		VERA_NOT_IMPLEMENTED;
	} else {
		result[0][0] = static_cast<T>(2.) / (right - left);
		result[1][1] = static_cast<T>(2.) / (top - bottom);
		result[2][2] = -static_cast<T>(1.);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);
	}

	return result;
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, default_major, T, packed_highp> perspective(
	radian fovy,
	T      aspect,
	T      z_near,
	T      z_far
) {
	T const tan_half_fovy = tan(fovy / static_cast<T>(2.));

	matrix_base<4, 4, default_major, T, packed_highp> result(static_cast<T>(0.));

	if constexpr (default_major == row_major) {
		VERA_NOT_IMPLEMENTED;
	} else {
		result[0][0] = static_cast<T>(1.) / (aspect * tan_half_fovy);
		result[1][1] = static_cast<T>(1.) / (tan_half_fovy);
		result[2][2] = z_far / (z_far - z_near);
		result[2][3] = static_cast<T>(1.);
		result[3][2] = -(z_far * z_near) / (z_far - z_near);
	}

	return result;
}

// detail/func_geometric.inl
//using faceforward;
//using reflect;
//using refract;

//// gtc/epsilon.hpp
//using epsilonEqual;
//using epsilonNotEqual;
//
//// gtc/noise.hpp
//using perlin;
//using simplex;
//
//// gtc/random.hpp
//using linearRand;
//using gaussRand;
//using circularRand;
//using sphericalRand;
//using diskRand;
//using ballRand;

VERA_NAMESPACE_END