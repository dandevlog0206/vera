#pragma once

#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

class QuadraticBezier2D
{
public:
	VERA_CONSTEXPR QuadraticBezier2D() VERA_NOEXCEPT :
		m_p0(),
		m_p1(),
		m_p2() {}

	VERA_CONSTEXPR QuadraticBezier2D(const float2& p0, const float2& p1, const float2& p2) VERA_NOEXCEPT :
		m_p0(p0),
		m_p1(p1),
		m_p2(p2) {}

	VERA_NODISCARD VERA_CONSTEXPR float2 operator()(float t) const VERA_NOEXCEPT
	{
		return quadratic(m_p0, m_p1, m_p2, t);
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

	VERA_NODISCARD VERA_CONSTEXPR const float2& p2() const VERA_NOEXCEPT
	{
		return m_p2;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2& p2() VERA_NOEXCEPT
	{
		return m_p2;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2 direction(float t) const VERA_NOEXCEPT
	{
		return 2.0f * (1.0f - t) * (m_p1 - m_p0) + 2.0f * t * (m_p2 - m_p1);
	}

	VERA_NODISCARD VERA_CONSTEXPR float2 secondDerivative() const VERA_NOEXCEPT
	{
		return 2.0f * (m_p2 - 2.0f * m_p1 + m_p0);
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
		return fabsf(cross(m_p2 - m_p0, m_p1 - m_p0)) <= epsilon;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const QuadraticBezier2D& rhs) const VERA_NOEXCEPT
	{
		return m_p0 == rhs.m_p0 && m_p1 == rhs.m_p1 && m_p2 == rhs.m_p2;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const QuadraticBezier2D& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float2 m_p0;
	float2 m_p1;
	float2 m_p2;
};

class QuadraticBezier3D
{
public:
	VERA_CONSTEXPR QuadraticBezier3D() VERA_NOEXCEPT :
		m_p0(),
		m_p1(),
		m_p2() {}

	VERA_CONSTEXPR QuadraticBezier3D(const float3& p0, const float3& p1, const float3& p2) VERA_NOEXCEPT :
		m_p0(p0),
		m_p1(p1),
		m_p2(p2) {}

	VERA_NODISCARD VERA_CONSTEXPR float3 operator()(float t) const VERA_NOEXCEPT
	{
		return quadratic(m_p0, m_p1, m_p2, t);
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 evaluate(float t) const VERA_NOEXCEPT
	{
		return (*this)(t);
	}

	VERA_NODISCARD VERA_CONSTEXPR const float3& p0() const VERA_NOEXCEPT
	{
		return m_p0;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3& p0() VERA_NOEXCEPT
	{
		return m_p0;
	}

	VERA_NODISCARD VERA_CONSTEXPR const float3& p1() const VERA_NOEXCEPT
	{
		return m_p1;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3& p1() VERA_NOEXCEPT
	{
		return m_p1;
	}

	VERA_NODISCARD VERA_CONSTEXPR const float3& p2() const VERA_NOEXCEPT
	{
		return m_p2;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3& p2() VERA_NOEXCEPT
	{
		return m_p2;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 direction(float t) const VERA_NOEXCEPT
	{
		return 2.0f * (1.0f - t) * (m_p1 - m_p0) + 2.0f * t * (m_p2 - m_p1);
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 secondDerivative() const VERA_NOEXCEPT
	{
		return 2.0f * (m_p2 - 2.0f * m_p1 + m_p0);
	}

	VERA_NODISCARD VERA_CONSTEXPR float length(size_t segments = 10) const VERA_NOEXCEPT
	{
		float length = 0.0f;
		float3 prev_point = m_p0;
		
		for (size_t i = 1; i <= segments; ++i) {
			float  t     = static_cast<float>(i) / static_cast<float>(segments);
			float3 point = (*this)(t);
			
			length    += ::vr::length(point - prev_point);
			prev_point = point;
		}
		
		return length;
	}

	VERA_NODISCARD VERA_INLINE bool isLinear(float epsilon = 1e-6f) const VERA_NOEXCEPT
	{
		return fabsf(::vr::length(cross(m_p2 - m_p0, m_p1 - m_p0))) <= epsilon;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const QuadraticBezier3D& rhs) const VERA_NOEXCEPT
	{
		return m_p0 == rhs.m_p0 && m_p1 == rhs.m_p1 && m_p2 == rhs.m_p2;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const QuadraticBezier3D& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float3 m_p0;
	float3 m_p1;
	float3 m_p2;
};

VERA_NAMESPACE_END
