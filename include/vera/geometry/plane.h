#pragma once

#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

class Plane
{
public:
	VERA_CONSTEXPR Plane() VERA_NOEXCEPT :
		m_normal(),
		m_k() {}

	VERA_CONSTEXPR Plane(const float3& normal, const float3& pos) VERA_NOEXCEPT :
		m_normal(normalize(normal)),
		m_k(-dot(m_normal, pos)) {}

	VERA_NODISCARD VERA_CONSTEXPR float operator()(const float3& pos) const VERA_NOEXCEPT
	{
		return dot(m_normal, pos) + m_k;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 normal() const VERA_NOEXCEPT
	{
		return m_normal;
	}

private:
	float3 m_normal;
	float  m_k;
};

VERA_NAMESPACE_END