#pragma once

#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

class Sphere
{
public:
	VERA_CONSTEXPR Sphere() VERA_NOEXCEPT :
		m_pos(0.f, 0.f, 0.f),
		m_radius(1.f) {}

	VERA_CONSTEXPR Sphere(float pos_x, float pos_y, float pos_z, float radius) VERA_NOEXCEPT :
		m_pos(pos_x, pos_y, pos_z),
		m_radius(radius) {}

	VERA_CONSTEXPR Sphere(const float3& pos, float radius) VERA_NOEXCEPT :
		m_pos(pos),
		m_radius(radius) {}

	VERA_NODISCARD VERA_CONSTEXPR const float3& pos() const VERA_NOEXCEPT
	{
		return m_pos;
	}

	VERA_NODISCARD VERA_CONSTEXPR float radius() const VERA_NOEXCEPT
	{
		return m_radius;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contains(const float3& point) const VERA_NOEXCEPT
	{
		return length(point - m_pos) <= m_radius;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Sphere& rhs) const VERA_NOEXCEPT
	{
		return m_pos == rhs.m_pos && m_radius == rhs.m_radius;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Sphere& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float3 m_pos;
	float  m_radius;
};

VERA_NAMESPACE_END
