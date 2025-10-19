#pragma once

#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

class QuadraticBezier2D
{
public:
	VERA_CONSTEXPR QuadraticBezier2D() VERA_NOEXCEPT :
		m_p0(),
		m_c(),
		m_p1() {}

	VERA_CONSTEXPR QuadraticBezier2D(const float2& p0, const float2& c, const float2& p1) VERA_NOEXCEPT :
		m_p0(p0),
		m_c(c),
		m_p1(p1) {}

	VERA_NODISCARD VERA_CONSTEXPR float2 operator()(float t) const VERA_NOEXCEPT
	{
		float u = 1.0f - t;
		return u * u * m_p0 + 2.0f * u * t * m_c + t * t * m_p1;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2 evaluate(float t) const VERA_NOEXCEPT
	{
		return (*this)(t);
	}

	VERA_NODISCARD VERA_CONSTEXPR const float2& p0() const VERA_NOEXCEPT
	{
		return m_p0;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2& p0() VERA_NOEXCEPT
	{
		return m_p0;
	}

	VERA_NODISCARD VERA_CONSTEXPR const float2& p1() const VERA_NOEXCEPT
	{
		return m_p1;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2& p1() VERA_NOEXCEPT
	{
		return m_p1;
	}

	VERA_NODISCARD VERA_CONSTEXPR const float2& controlPoint() const VERA_NOEXCEPT
	{
		return m_c;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2& controlPoint() VERA_NOEXCEPT
	{
		return m_c;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2 derivative(float t) const VERA_NOEXCEPT
	{
		return 2.0f * (1.0f - t) * (m_c - m_p0) + 2.0f * t * (m_p1 - m_c);
	}

	VERA_NODISCARD VERA_CONSTEXPR float2 secondDerivative() const VERA_NOEXCEPT
	{
		return 2.0f * (m_p1 - 2.0f * m_c + m_p0);
	}

	VERA_NODISCARD VERA_CONSTEXPR float length(size_t segments = 10) const VERA_NOEXCEPT
	{
		float length = 0.0f;
		float2 prev_point = m_p0;
		
		for (size_t i = 1; i <= segments; ++i) {
			float  t     = static_cast<float>(i) / static_cast<float>(segments);
			float2 point = (*this)(t);
			
			length    += ::vr::length(point - prev_point);
			prev_point = point;
		}
		
		return length;
	}

	VERA_NODISCARD VERA_INLINE bool isLinear(float epsilon = 1e-6f) const VERA_NOEXCEPT
	{
		float2 ab    = m_p1 - m_p0;
		float2 ac    = m_c  - m_p0;
		float  cross = ab.x * ac.y - ab.y * ac.x;
		
		return fabsf(cross) <= epsilon;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const QuadraticBezier2D& rhs) const VERA_NOEXCEPT
	{
		return m_p0 == rhs.m_p0 && m_c == rhs.m_c && m_p1 == rhs.m_p1;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const QuadraticBezier2D& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float2 m_p0;
	float2 m_c;
	float2 m_p1;
};

VERA_NAMESPACE_END
