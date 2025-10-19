#pragma once

#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

class Line
{
public:
	VERA_CONSTEXPR Line() VERA_NOEXCEPT :
		m_pos(0.f, 0.f, 0.f),
		m_dir(0.f, 0.f, 0.f) {}

	VERA_CONSTEXPR Line(float pos_x, float pos_y, float pos_z, float dir_x, float dir_y, float dir_z) VERA_NOEXCEPT :
		m_pos(pos_x, pos_y, pos_z),
		m_dir(normalize(float3(dir_x, dir_y, dir_z))) {}

	VERA_CONSTEXPR Line(const float3& pos, const float3& dir) VERA_NOEXCEPT :
		m_pos(pos),
		m_dir(normalize(dir)) {}
	
	VERA_NODISCARD VERA_CONSTEXPR const float3& pos() const VERA_NOEXCEPT
	{
		return m_pos;
	}
	
	VERA_NODISCARD VERA_CONSTEXPR const float3& dir() const VERA_NOEXCEPT
	{
		return m_dir;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 operator()(float t) const VERA_NOEXCEPT
	{
		return m_pos + m_dir * t;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Line& rhs) const VERA_NOEXCEPT
	{
		return m_pos == rhs.m_pos && m_dir == rhs.m_dir;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Line& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	float3 m_pos;
	float3 m_dir;
};

VERA_NAMESPACE_END