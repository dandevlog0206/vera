#pragma once

#include "camera.h"

VERA_NAMESPACE_BEGIN

class Flycam : public Camera
{
public:
	Flycam();
	Flycam(const float3& pos, const float3& look);

	void updateEvent(const os::WindowEvent& e, float dt) override;
	void updateAsync(float dt) override;

	VERA_NODISCARD float3 getPosition() const VERA_NOEXCEPT override;
	VERA_NODISCARD float3 getLook() const VERA_NOEXCEPT override;
	VERA_NODISCARD float3 getDirection() const VERA_NOEXCEPT override;
	VERA_NODISCARD float3 getUp() const VERA_NOEXCEPT override;

	void setVelocity(float velocity) VERA_NOEXCEPT;

	VERA_NODISCARD float4x4 getMatrix() const VERA_NOEXCEPT override;

private:
	void reinitOrientation();
	void setReferenceFrame();

private:
	float3 m_up;
	float3 m_pos;
	float3 m_look;
	float3 m_dir;
	float3 m_u;
	float3 m_v;
	float3 m_w;
	float  m_latitude;
	float  m_longitude;
	float  m_velocity;
	float  m_mouse_speed;
	bool   m_is_tracking;
};

VERA_NAMESPACE_END
