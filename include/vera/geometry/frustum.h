#pragma once

#include "plane.h"

VERA_NAMESPACE_BEGIN

class Frustum
{
public:
	VERA_CONSTEXPR Frustum() VERA_NOEXCEPT :
		m_pos(),
		m_dir(),
		m_up(),
		m_near(),
		m_far(),
		m_vfov(),
		m_aspect() {}

	VERA_NODISCARD VERA_CONSTEXPR float3 position() const VERA_NOEXCEPT
	{
		return m_pos;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 direction() const VERA_NOEXCEPT
	{
		return m_dir;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 up() const VERA_NOEXCEPT
	{
		return m_up;
	}

	VERA_NODISCARD VERA_CONSTEXPR float near() const VERA_NOEXCEPT
	{
		return m_near;
	}

	VERA_NODISCARD VERA_CONSTEXPR float far() const VERA_NOEXCEPT
	{
		return m_far;
	}

	VERA_NODISCARD VERA_CONSTEXPR float vfov() const VERA_NOEXCEPT
	{
		return m_vfov;
	}

	VERA_NODISCARD VERA_CONSTEXPR float aspect() const VERA_NOEXCEPT
	{
		return m_aspect;
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane farPlane() const VERA_NOEXCEPT
	{
		return Plane(m_dir, m_pos + m_dir * m_far);
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane nearPlane() const VERA_NOEXCEPT
	{
		return Plane(m_dir, m_pos + m_dir * m_near);
	}

private:
	float3 m_pos;
	float3 m_dir;
	float3 m_up;
	float  m_near;
	float  m_far;
	float  m_vfov;
	float  m_aspect;
};

VERA_NAMESPACE_END
