#pragma once

#include "vector_types.h"
#include "matrix_types.h"
#include <cmath>

VERA_NAMESPACE_BEGIN

template <class T>
VERA_NODISCARD VERA_CONSTEXPR float length(const vector_base<2, T>& v) VERA_NOEXCEPT
{
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR float length(const vector_base<3, T>& v) VERA_NOEXCEPT
{
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR float length(const vector_base<4, T>& v) VERA_NOEXCEPT
{
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

template <size_t Dim, class T>
VERA_NODISCARD VERA_CONSTEXPR float distance(const vector_base<Dim, T>& lhs, const vector_base<Dim, T>& rhs) VERA_NOEXCEPT
{
	return length(rhs - lhs);
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR float dot(const vector_base<2, T>& lhs, const vector_base<2, T>& rhs) VERA_NOEXCEPT
{
	return lhs.x * rhs.x + lhs.y * rhs.y;
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR float dot(const vector_base<3, T>& lhs, const vector_base<3, T>& rhs) VERA_NOEXCEPT
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR float dot(const vector_base<4, T>& lhs, const vector_base<4, T>& rhs) VERA_NOEXCEPT
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

template <class T>
VERA_NODISCARD VERA_CONSTEXPR vector_base<3, T> cross(const vector_base<3, T>& lhs, const vector_base<3, T>& rhs) VERA_NOEXCEPT
{
	return {
		lhs.y * rhs.z - rhs.y * lhs.z,
		lhs.z * rhs.x - rhs.z * lhs.x,
		lhs.x * rhs.y - rhs.x * lhs.y };
}

// TODO: glm::normalize???
//template <size_t Dim, class T>
//VERA_NODISCARD VERA_CONSTEXPR vector_base<Dim, T> normalize(const vector_base<Dim, T>& v) VERA_NOEXCEPT
//{
//	return v / length(v);
//}

template<typename T>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, T> lookAt(const vector_base<3, T>& eye, const vector_base<3, T>& center, const vector_base<3, T>& up)
{
	const vector_base<3, T> f(normalize(center - eye));
	const vector_base<3, T> s(normalize(cross(up, f)));
	const vector_base<3, T> u(cross(f, s));

	matrix_base<4, 4, T> result(1.f);
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

	return result;
}

template<typename T>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, T> ortho(T left, T right, T bottom, T top)
{
	matrix_base<4, 4, T> result(static_cast<T>(1.));

	result[0][0] = static_cast<T>(2.) / (right - left);
	result[1][1] = static_cast<T>(2.) / (top - bottom);
	result[2][2] = -static_cast<T>(1.);
	result[3][0] = -(right + left) / (right - left);
	result[3][1] = -(top + bottom) / (top - bottom);

	return result;
}

template<typename T>
VERA_NODISCARD VERA_CONSTEXPR matrix_base<4, 4, T> perspective(T fovy, T aspect, T z_near, T z_far)
{
	assert(abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0.));

	T const tan_half_fovy = tan(fovy / static_cast<T>(2.));

	matrix_base<4, 4, T> result(static_cast<T>(0.));
	result[0][0] = static_cast<T>(1.) / (aspect * tan_half_fovy);
	result[1][1] = static_cast<T>(1.) / (tan_half_fovy);
	result[2][2] = z_far / (z_far - z_near);
	result[2][3] = static_cast<T>(1.);
	result[3][2] = -(z_far * z_near) / (z_far - z_near);

	return result;
}

// detail/func_geometric.inl
//using glm::faceforward;
//using glm::reflect;
//using glm::refract;

//// gtc/epsilon.hpp
//using glm::epsilonEqual;
//using glm::epsilonNotEqual;
//
//// gtc/noise.hpp
//using glm::perlin;
//using glm::simplex;
//
//// gtc/random.hpp
//using glm::linearRand;
//using glm::gaussRand;
//using glm::circularRand;
//using glm::sphericalRand;
//using glm::diskRand;
//using glm::ballRand;

VERA_NAMESPACE_END