#pragma once

#include "camera.h"

VERA_NAMESPACE_BEGIN

class Arcball : public Camera
{
public:
	Arcball();
	Arcball(const float3& pos, const float3& look_at);

	void updateEvent(const os::WindowEvent& e, float dt) override;

	VERA_NODISCARD float3 getPosition() const VERA_NOEXCEPT override;
	VERA_NODISCARD float3 getLook() const VERA_NOEXCEPT override;
	VERA_NODISCARD float3 getDirection() const VERA_NOEXCEPT override;
	VERA_NODISCARD float3 getUp() const VERA_NOEXCEPT override;

	void setDistance(float dist) VERA_NOEXCEPT;
	void setMinDistance(float dist) VERA_NOEXCEPT;
	void setMaxDistance(float dist) VERA_NOEXCEPT;
	void setDistanceLimits(float min_dist, float max_dist) VERA_NOEXCEPT;
	void setMinLongitude(float min_long) VERA_NOEXCEPT;
	void setMaxLongitude(float max_long) VERA_NOEXCEPT;
	void setLongitudeLimits(float min_long, float max_long) VERA_NOEXCEPT;

	VERA_NODISCARD float4x4 getMatrix() const VERA_NOEXCEPT override;

private:
	void updateAngles() VERA_NOEXCEPT;
	void updatePos() VERA_NOEXCEPT;

private:
	float3 m_center;
	float3 m_pos;
	float3 m_dir;
	float  m_distance;
	float  m_distance_min;
	float  m_distance_max;
	float  m_latitude;
	float  m_longitude;
	float  m_longitude_min;
	float  m_longitude_max;
	bool   m_is_tracking;
};

VERA_NAMESPACE_END