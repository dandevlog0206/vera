#pragma once

#include "camera.h"

VERA_NAMESPACE_BEGIN

class Arcball : public Camera
{
public:
	Arcball();
	Arcball(const float3& pos, const float3& look_at);

	void updateEvent(const os::WindowEvent& e, float dt) override;

	float4x4 getMatrix() const override;

private:
	void updateAngles();
	void updatePos();

private:
	float3 m_center;
	float3 m_pos;
	float3 m_dir;
	float  m_distance;
	float  m_latitude;
	float  m_longitude;
	bool   m_is_tracking;
};

VERA_NAMESPACE_END