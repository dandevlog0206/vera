#pragma once

#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

class LineSegment
{
public:
	VERA_CONSTEXPR LineSegment() VERA_NOEXCEPT :
		m_start(0.f, 0.f, 0.f),
		m_end(0.f, 0.f, 0.f) {}

	VERA_CONSTEXPR LineSegment(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z) VERA_NOEXCEPT :
		m_start(start_x, start_y, start_z),
		m_end(end_x, end_y, end_z) {}
	
	VERA_CONSTEXPR LineSegment(const float3& start, const float3& end) VERA_NOEXCEPT :
		m_start(start),
		m_end(end) {}
	
	VERA_NODISCARD VERA_CONSTEXPR float3 start() const VERA_NOEXCEPT
	{
		return m_start;
	}
	
	VERA_NODISCARD VERA_CONSTEXPR float3 end() const VERA_NOEXCEPT
	{
		return m_end;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 direction() const VERA_NOEXCEPT
	{
		return normalize(m_end - m_start);
	}

	VERA_NODISCARD VERA_CONSTEXPR float length() const VERA_NOEXCEPT
	{
		return ::vr::length(m_end - m_start);
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 operator()(float t) const VERA_NOEXCEPT
	{
		return m_start + (m_end - m_start) * t;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const LineSegment& rhs) const VERA_NOEXCEPT
	{
		return m_start == rhs.m_start && m_end == rhs.m_end;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const LineSegment& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float3 m_start;
	float3 m_end;
};

VERA_NAMESPACE_END