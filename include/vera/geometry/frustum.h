#pragma once

#include "plane.h"

VERA_NAMESPACE_BEGIN

enum class FrustumPlaneType VERA_ENUM
{
	Near   = 0,
	Far    = 1,
	Left   = 2,
	Right  = 3,
	Top    = 4,
	Bottom = 5
};

struct FrustumPlaneSet
{
	Plane near;
	Plane far;
	Plane left;
	Plane right;
	Plane top;
	Plane bottom;
};

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

	VERA_CONSTEXPR Frustum(
		const float3& pos,
		const float3& dir,
		const float3& up,
		float near,
		float far,
		float vfov,
		float aspect
	) VERA_NOEXCEPT :
		m_pos(pos),
		m_dir(normalize(dir)),
		m_up(normalize(up)),
		m_near(near),
		m_far(far),
		m_vfov(vfov),
		m_aspect(aspect) {}

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

	VERA_NODISCARD VERA_CONSTEXPR FrustumPlaneSet getPlaneSet() const VERA_NOEXCEPT
	{
		return FrustumPlaneSet{
			nearPlane(),
			farPlane(),
			leftPlane(),
			rightPlane(),
			topPlane(),
			bottomPlane()
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane getPlane(FrustumPlaneType plane) const VERA_NOEXCEPT
	{
		switch (plane) {
		case FrustumPlaneType::Near: return nearPlane();
		case FrustumPlaneType::Far:  return farPlane();
		case FrustumPlaneType::Left: return leftPlane();
		case FrustumPlaneType::Right:return rightPlane();
		case FrustumPlaneType::Top:  return topPlane();
		case FrustumPlaneType::Bottom:return bottomPlane();
		}

		return Plane{};
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane nearPlane() const VERA_NOEXCEPT
	{
		return Plane{ m_dir, m_pos + m_dir * m_near };
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane farPlane() const VERA_NOEXCEPT
	{
		return Plane{ m_dir, m_pos + m_dir * m_far };
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane leftPlane() const VERA_NOEXCEPT
	{
		float3 right       = normalize(cross(m_dir, m_up));
		float3 near_center = m_pos + m_dir * m_near;
		float3 near_left   = near_center - right * (m_near * tanf(m_vfov * 0.5f) * m_aspect);
		float3 normal      = normalize(cross(normalize(near_left - m_pos), m_up));
		return Plane{ normal, m_pos };
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane rightPlane() const VERA_NOEXCEPT
	{
		float3 right       = normalize(cross(m_dir, m_up));
		float3 near_center = m_pos + m_dir * m_near;
		float3 near_right  = near_center + right * (m_near * tanf(m_vfov * 0.5f) * m_aspect);
		float3 normal      = normalize(cross(m_up, normalize(near_right - m_pos)));
		return Plane{ normal, m_pos };
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane topPlane() const VERA_NOEXCEPT
	{
		float3 right       = normalize(cross(m_dir, m_up));
		float3 up          = normalize(cross(right, m_dir));
		float3 near_center = m_pos + m_dir * m_near;
		float3 near_top    = near_center + up * (m_near * tanf(m_vfov * 0.5f));
		float3 normal      = normalize(cross(right, normalize(near_top - m_pos)));
		return Plane{ normal, m_pos };
	}

	VERA_NODISCARD VERA_CONSTEXPR Plane bottomPlane() const VERA_NOEXCEPT
	{
		float3 right       = normalize(cross(m_dir, m_up));
		float3 up          = normalize(cross(right, m_dir));
		float3 near_center = m_pos + m_dir * m_near;
		float3 near_bottom = near_center - up * (m_near * tanf(m_vfov * 0.5f));
		float3 normal      = normalize(cross(normalize(near_bottom - m_pos), right));
		return Plane{ normal, m_pos };
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Frustum& rhs) const VERA_NOEXCEPT
	{
		return
			m_pos    == rhs.m_pos &&
			m_dir    == rhs.m_dir &&
			m_up     == rhs.m_up &&
			m_near   == rhs.m_near &&
			m_far    == rhs.m_far &&
			m_vfov   == rhs.m_vfov &&
			m_aspect == rhs.m_aspect;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Frustum& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
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
