#pragma once

#include "../core/assertion.h"
#include "../math/vector_types.h"
#include <limits>
#include <cmath>

VERA_NAMESPACE_BEGIN

class AABB
{
public:
	VERA_CONSTEXPR AABB() VERA_NOEXCEPT :
		m_min(FLT_MAX, FLT_MAX, FLT_MAX ),
		m_max(-FLT_MAX, -FLT_MAX, -FLT_MAX ) {}

	VERA_CONSTEXPR AABB(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z) VERA_NOEXCEPT :
		m_min(min_x, min_y, min_z),
		m_max(max_x, max_y, max_z)
	{
		VERA_ASSERT(m_min.x <= m_max.x && m_min.y <= m_max.y && m_min.z <= m_max.z);
	}
	
	VERA_CONSTEXPR AABB(const float3& min, const float3& max) VERA_NOEXCEPT :
		m_min(min),
		m_max(max)
	{
		VERA_ASSERT(m_min.x <= m_max.x && m_min.y <= m_max.y && m_min.z <= m_max.z);
	}

	VERA_NODISCARD VERA_CONSTEXPR const float3& min() const VERA_NOEXCEPT
	{
		return m_min;
	}

	VERA_NODISCARD VERA_CONSTEXPR const float3& max() const VERA_NOEXCEPT
	{
		return m_max;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 center() const VERA_NOEXCEPT
	{
		return (m_min + m_max) * 0.5f;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool intersect(const AABB& rhs) const VERA_NOEXCEPT
	{
		return 
			!(m_max.x < rhs.m_min.x || rhs.m_max.x < m_min.x) &&
			!(m_max.y < rhs.m_min.y || rhs.m_max.y < m_min.y)&&
			!(m_max.z < rhs.m_min.z || rhs.m_max.z < m_min.z);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const float3& p) const VERA_NOEXCEPT
	{
		return
			m_min.x <= p.x && p.x <= m_max.x &&
			m_min.y <= p.y && p.y <= m_max.y &&
			m_min.z <= p.z && p.z <= m_max.z;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const AABB& rhs) const VERA_NOEXCEPT
	{
		return
			m_min.x <= rhs.m_min.x && rhs.m_max.x <= m_max.x &&
			m_min.y <= rhs.m_min.y && rhs.m_max.y <= m_max.y &&
			m_min.z <= rhs.m_min.z && rhs.m_max.z <= m_max.z;
	}

	VERA_CONSTEXPR void expand(const float3& point) VERA_NOEXCEPT
	{
		m_min.x = fminf(m_min.x, point.x);
		m_min.y = fminf(m_min.y, point.y);
		m_min.z = fminf(m_min.z, point.z);
		m_max.x = fmaxf(m_max.x, point.x);
		m_max.y = fmaxf(m_max.y, point.y);
		m_max.z = fmaxf(m_max.z, point.z);
	}

	VERA_CONSTEXPR void expand(const AABB& aabb) VERA_NOEXCEPT
	{
		m_min.x = fminf(m_min.x, aabb.m_min.x);
		m_min.y = fminf(m_min.y, aabb.m_min.y);
		m_min.z = fminf(m_min.z, aabb.m_min.z);
		m_max.x = fmaxf(m_max.x, aabb.m_max.x);
		m_max.y = fmaxf(m_max.y, aabb.m_max.y);
		m_max.z = fmaxf(m_max.z, aabb.m_max.z);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const AABB& rhs) const VERA_NOEXCEPT
	{
		return m_min == rhs.m_min && m_max == rhs.m_max;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const AABB& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float3 m_min;
	float3 m_max;
};

VERA_NAMESPACE_END