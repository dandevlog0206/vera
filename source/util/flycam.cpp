#include "../../include/vera/util/flycam.h"

#include "../../include/vera/math/vector_math.h"
#include "../../include/vera/math/constant.h"
#include "../../include/vera/os/window.h"
#include <algorithm>
#include <cmath>

VERA_NAMESPACE_BEGIN

Flycam::Flycam() :
	m_up(0.f, 1.f, 0.f),
	m_pos(0.f, 0.f, 0.f),
	m_look(0.f, 0.f, 1.f),
	m_dir(normalize(m_look - m_pos)),
	m_u(1.f, 0.f, 0.f),
	m_v(0.f, 1.f, 0.f),
	m_w(0.f, 0.f, 1.f),
	m_latitude(0.f),
	m_longitude(0.f),
	m_velocity(1.f),
	m_mouse_speed(0.1f) {}

Flycam::Flycam(const float3& pos, const float3& look) :
	m_up(0.f, 1.f, 0.f),
	m_pos(pos),
	m_look(look),
	m_dir(normalize(m_look - m_pos)),
	m_u(),
	m_v(),
	m_w(),
	m_latitude(0.f),
	m_longitude(0.f),
	m_velocity(1.f),
	m_mouse_speed(0.1f),
	m_is_tracking(false)
{
	setReferenceFrame();
	reinitOrientation();
}

void Flycam::updateEvent(const os::WindowEvent& e, float dt)
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

		const float pi   = numeric_const<float>::pi;
		const float pi_2 = 2.f * pi;

		auto  args   = e.get_if<os::MouseMoveArgs>();
		float dx     = static_cast<float>(args.delta.x);
		float dy     = static_cast<float>(args.delta.y);
		float aspect = args.window->getAspect();

		m_longitude = std::clamp(m_longitude + 0.005f * dy, -pi_2 + 1e-3f, pi_2 - 1e-3f);
		m_latitude  = fmodf(m_latitude + 0.005f / aspect * dx, 2.f * pi);

		float3 local_dir;
		local_dir.x = -cosf(m_longitude) * sinf(m_latitude);
		local_dir.y = sinf(m_longitude);
		local_dir.z = cosf(m_longitude) * cosf(m_latitude);

		m_dir = m_u * local_dir.x + m_v * local_dir.y + m_w * local_dir.z;

		m_look = m_pos + m_dir * 100.f;

		reinitOrientation();
	} break;
	}
}

void Flycam::updateAsync(float dt)
{
	if (os::Keyboard::isDown(os::Scancode::W) || os::Keyboard::isDown(os::Scancode::Up)) {
		float3 dir = normalize(float3(m_dir.x, 0.f, m_dir.z));

		m_pos  = m_pos + dir * m_velocity * dt;
		m_look = m_look + dir * m_velocity * dt;
	}
	if (os::Keyboard::isDown(os::Scancode::A) || os::Keyboard::isDown(os::Scancode::Left)) {
		float3 dir = normalize(cross(m_dir, m_up));

		m_pos  = m_pos + dir * m_velocity * dt;
		m_look = m_look + dir * m_velocity * dt;
	}
	if (os::Keyboard::isDown(os::Scancode::S) || os::Keyboard::isDown(os::Scancode::Down)) {
		float3 dir = -normalize(float3(m_dir.x, 0.f, m_dir.z));

		m_pos  = m_pos + dir * m_velocity * dt;
		m_look = m_look + dir * m_velocity * dt;
	}
	if (os::Keyboard::isDown(os::Scancode::D) || os::Keyboard::isDown(os::Scancode::Right)) {
		float3 dir = -normalize(cross(m_dir, m_up));

		m_pos  = m_pos + dir * m_velocity * dt;
		m_look = m_look + dir * m_velocity * dt;
	}
	if (os::Keyboard::isDown(os::Scancode::LShift)) {
		m_pos  = m_pos - m_up * m_velocity * dt;
		m_look = m_look - m_up * m_velocity * dt;
	}
	if (os::Keyboard::isDown(os::Scancode::Space)) {
		m_pos  = m_pos + m_up * m_velocity * dt;
		m_look = m_look + m_up * m_velocity * dt;
	}
}

float3 Flycam::getPosition() const
{
	return m_pos;
}

float4x4 Flycam::getMatrix() const
{
	return lookAt(m_pos, m_look, m_v);
}

void Flycam::reinitOrientation()
{
	m_w = normalize(m_look - m_pos);
	m_u = normalize(cross(m_up, m_w));
	m_v = normalize(cross(m_w, m_u));

	m_latitude  = 0.0f;
	m_longitude = 0.0f;
}

void Flycam::setReferenceFrame()
{
	m_latitude  = atan2f(m_dir.z, m_dir.x);
	m_longitude = atan2f(m_dir.y, sqrtf(m_dir.x * m_dir.x + m_dir.z * m_dir.z));

	// m_latitude  = asin(dot(m_dir, m_w));
	// m_longitude = atan2(dot(m_dir, m_u), dot(m_dir, m_v));
}

VERA_NAMESPACE_END
