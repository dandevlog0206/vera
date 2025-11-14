#pragma once

#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

class Circle
{
public:
	VERA_CONSTEXPR Circle() VERA_NOEXCEPT :
		m_center(),
		m_radius(1.f) {}

	VERA_CONSTEXPR Circle(const float2& center, float radius) VERA_NOEXCEPT :
		m_center(center),
		m_radius(radius) {}

	VERA_CONSTEXPR Circle(float center_x, float center_y, float radius) VERA_NOEXCEPT :
		m_center(center_x, center_y),
		m_radius(radius) {}

	VERA_NODISCARD VERA_CONSTEXPR const float2& center() const VERA_NOEXCEPT
	{
		return m_center;
	}

	VERA_NODISCARD VERA_CONSTEXPR float2& center() VERA_NOEXCEPT
	{
		return m_center;
	}

	VERA_NODISCARD VERA_CONSTEXPR float radius() const VERA_NOEXCEPT
	{
		return m_radius;
	}

	VERA_NODISCARD VERA_CONSTEXPR float& radius() VERA_NOEXCEPT
	{
		return m_radius;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contains(const float2& point) const VERA_NOEXCEPT
	{
		return length(point - m_center) <= m_radius;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Circle& rhs) const VERA_NOEXCEPT
	{
		return m_center == rhs.m_center && m_radius == rhs.m_radius;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Circle& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float2 m_center;
	float  m_radius;
};

VERA_NAMESPACE_END
