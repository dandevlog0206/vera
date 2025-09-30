#include "../../include/vera/util/arcball.h"

#include "../../include/vera/math/constant.h"
#include "../../include/vera/math/vector_math.h"
#include "../../include/vera/os/window.h"
#include <algorithm>
#include <cmath>

VERA_NAMESPACE_BEGIN

Arcball::Arcball()
{

}

Arcball::Arcball(const float3& pos, const float3& look_at) :
	m_center(look_at),
	m_pos(pos),
	m_dir(normalize(pos - look_at)),
	m_distance(length(pos - look_at)),
	m_latitude(0.f),
	m_longitude(0.f),
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

		const float pi     = numeric_const<float>::pi;
		const float pi_2   = 2.f * pi;

		auto  args   = e.get_if<os::MouseMoveArgs>();
		float dx     = args.delta.x;
		float dy     = args.delta.y;
		float aspect = args.window->getAspect();

		m_longitude = std::clamp(m_longitude - 0.005f * dy, -pi_2 + 1e-3f, pi_2 - 1e-3f);
		m_latitude  = fmodf(m_latitude + 0.005f / aspect * dx, 2.f * pi);

		updatePos();
	} break;
	case os::WindowEventType::MouseWheel:
		if (0 < e.get_if<os::MouseWheelArgs>().delta_y) {
			m_distance *= 1.05f;
		} else {
			m_distance /= 1.05f;
		}

		m_pos = m_center + m_distance * m_dir;
		break;
	}
}

float4x4 Arcball::getMatrix() const
{
	float3 right = cross(float3(0.f, 1.f, 0.f), -m_dir);
	float3 up    = cross(-m_dir, right);

	return lookAt(m_pos, m_center, up);
}

void Arcball::updateAngles()
{
	m_latitude  = atan2f(m_dir.z, m_dir.x);
	m_longitude = atan2f(m_dir.y, sqrtf(m_dir.x * m_dir.x + m_dir.z * m_dir.z));
}

void Arcball::updatePos()
{
	m_dir.x = cosf(m_longitude) * cosf(m_latitude);
	m_dir.y = sinf(m_longitude);
	m_dir.z = cosf(m_longitude) * sinf(m_latitude);

	m_pos = m_center + m_distance * m_dir;
}

VERA_NAMESPACE_END