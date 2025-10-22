#include "../../include/vera/util/arcball.h"

#include "../../include/vera/core/assertion.h"
#include "../../include/vera/math/constant.h"
#include "../../include/vera/math/vector_math.h"
#include "../../include/vera/os/window.h"
#include <algorithm>
#include <cmath>

VERA_NAMESPACE_BEGIN

Arcball::Arcball() :
	m_center(float3(0.f, 0.f, 0.f)),
	m_pos(float3(0.f, 0.f, 5.f)),
	m_dir(float3(0.f, 0.f, -1.f)),
	m_distance(5.f),
	m_distance_min(0.f),
	m_distance_max(FLT_MAX),
	m_latitude(0.f),
	m_longitude(0.f),
	m_longitude_min(to_radian(-90.f) + 1e-3f),
	m_longitude_max(to_radian(90.f) - 1e-3f),
	m_is_tracking(false) {}

Arcball::Arcball(const float3& pos, const float3& look_at) :
	m_center(look_at),
	m_pos(pos),
	m_dir(normalize(pos - look_at)),
	m_distance(length(pos - look_at)),
	m_distance_min(0.f),
	m_distance_max(FLT_MAX),
	m_latitude(0.f),
	m_longitude(0.f),
	m_longitude_min(to_radian(-90.f) + 1e-3f),
	m_longitude_max(to_radian(90.f) - 1e-3f),
	m_is_tracking(false)
{
	updateAngles();
}

void Arcball::updateEvent(const os::WindowEvent& e, float dt)
{
	switch (e.type()) {
	case os::WindowEventType::MouseButtonUp:
		if (e.get_if<os::MouseButtonArgs>().button == os::Mouse::LButton)
			m_is_tracking = false;
		break;
	case os::WindowEventType::MouseButtonDown:
		if (e.get_if<os::MouseButtonArgs>().button == os::Mouse::LButton)
			m_is_tracking = true;
		break;
	case os::WindowEventType::LostFocus:
		m_is_tracking = false;
		break;
	case os::WindowEventType::MouseMove: {
		if (!m_is_tracking) break;

		const float pi = numeric_const<float>::pi;

		auto  args   = e.get_if<os::MouseMoveArgs>();
		float dx     = static_cast<float>(args.delta.x);
		float dy     = static_cast<float>(args.delta.y);
		float aspect = args.window->getAspect();

		m_longitude = std::clamp(m_longitude - 0.005f * dy, m_longitude_min, m_longitude_max);
		m_latitude  = fmodf(m_latitude + 0.005f / aspect * dx, 2.f * pi);

		updatePos();
	} break;
	case os::WindowEventType::MouseWheel:
		if (0 < e.get_if<os::MouseWheelArgs>().delta_y) {
			m_distance = std::min(m_distance * 1.05f, m_distance_max);
		} else {
			m_distance = std::max(m_distance / 1.05f, m_distance_min);
		}

		m_pos = m_center + m_distance * m_dir;
		break;
	}
}

float3 Arcball::getPosition() const VERA_NOEXCEPT
{
	return m_pos;
}

float3 Arcball::getLook() const VERA_NOEXCEPT
{
	return m_center;
}

float3 Arcball::getDirection() const VERA_NOEXCEPT
{
	return -m_dir;
}

float3 Arcball::getUp() const VERA_NOEXCEPT
{
	return float3(0.f, 1.f, 0.f);
}

void Arcball::setDistance(float dist) VERA_NOEXCEPT
{
	m_distance = std::clamp(dist, m_distance_min, m_distance_max);
	updatePos();
}

void Arcball::setMinDistance(float dist) VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(0.f <= dist, "minimum distance must be non-negative");

	m_distance_min = std::max(dist, 0.f);
	m_distance_max = std::max(m_distance_max, m_distance_min);
	m_distance     = std::max(m_distance, m_distance_min);
	updatePos();
}

void Arcball::setMaxDistance(float dist) VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(0.f <= dist, "maximum distance must be non-negative");

	m_distance_max = dist, 0.f;
	m_distance_min = std::min(m_distance_min, m_distance_max);
	m_distance     = std::min(m_distance, m_distance_max);
	updatePos();
}

void Arcball::setDistanceLimits(float min_dist, float max_dist) VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(0.f <= min_dist && 0.f <= max_dist, "distances must be non-negative");
	VERA_ASSERT_MSG(min_dist <= max_dist, "minimum distance must be less than or equal to maximum distance");

	m_distance_min = min_dist;
	m_distance_max = max_dist;
	m_distance     = std::clamp(m_distance, m_distance_min, m_distance_max);
	updatePos();
}

void Arcball::setMinLongitude(float min_long) VERA_NOEXCEPT
{
	m_longitude_min = min_long;
	m_longitude     = std::clamp(m_longitude, m_longitude_min, m_longitude_max);
	updatePos();
}

void Arcball::setMaxLongitude(float max_long) VERA_NOEXCEPT
{
	m_longitude_max = max_long;
	m_longitude     = std::clamp(m_longitude, m_longitude_min, m_longitude_max);
	updatePos();
}

void Arcball::setLongitudeLimits(float min_long, float max_long) VERA_NOEXCEPT
{
	m_longitude_min = min_long;
	m_longitude_max = max_long;
	m_longitude     = std::clamp(m_longitude, m_longitude_min, m_longitude_max);
	updatePos();
}

float4x4 Arcball::getMatrix() const VERA_NOEXCEPT
{
	float3 right = cross(float3(0.f, 1.f, 0.f), -m_dir);
	float3 up    = cross(-m_dir, right);

	return lookAt(m_pos, m_center, up);
}

void Arcball::updateAngles() VERA_NOEXCEPT
{
	m_latitude  = atan2f(m_dir.z, m_dir.x);
	m_longitude = atan2f(m_dir.y, sqrtf(m_dir.x * m_dir.x + m_dir.z * m_dir.z));
}

void Arcball::updatePos() VERA_NOEXCEPT
{
	m_dir.x = cosf(m_longitude) * cosf(m_latitude);
	m_dir.y = sinf(m_longitude);
	m_dir.z = cosf(m_longitude) * sinf(m_latitude);

	m_pos = m_center + m_distance * m_dir;
}

VERA_NAMESPACE_END