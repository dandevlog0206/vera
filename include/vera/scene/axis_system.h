#pragma once

#include "../core/assertion.h"
#include "../math/vector_types.h"
#include "../math/vector_math.h"

VERA_NAMESPACE_BEGIN

enum class CoordinateSystem VERA_ENUM
{
	LeftHanded,
	RightHanded
};

enum class AxisDirection VERA_ENUM
{
	X      = 0,
	Y      = 1,
	Z      = 2,
	MinusX = 3,
	MinusY = 4,
	MinusZ = 5,

	PlusX = X,
	PlusY = Y,
	PlusZ = Z
};

class AxisSystem
{
public:
	VERA_CONSTEXPR AxisSystem() VERA_NOEXCEPT :
		m_coord_system(CoordinateSystem::LeftHanded),
		m_up_dir(AxisDirection::PlusY),
		m_front_dir(AxisDirection::PlusZ),
		m_up(0.f, 1.f, 0.f),
		m_front(0.f, 0.f, 1.f),
		m_left(-1.f, 0.f, 0.f),
		m_right(1.f, 0.f, 0.f) {}

	VERA_CONSTEXPR AxisSystem(CoordinateSystem system, AxisDirection up, AxisDirection front) VERA_NOEXCEPT :
		m_coord_system(system),
		m_up_dir(up),
		m_front_dir(front),
		m_up(),
		m_front(),
		m_left(),
		m_right()
	{
		VERA_ASSERT_MSG(static_cast<uint32_t>(up) % 3 != static_cast<uint32_t>(front) % 3,
			"up and front direction must be different component");

		switch (m_up_dir) {
		case AxisDirection::PlusX:  m_up = float3(1.f, 0.f, 0.f); break;
		case AxisDirection::PlusY:  m_up = float3(0.f, 1.f, 0.f); break;
		case AxisDirection::PlusZ:  m_up = float3(0.f, 0.f, 1.f); break;
		case AxisDirection::MinusX: m_up = float3(-1.f, 0.f, 0.f); break;
		case AxisDirection::MinusY: m_up = float3(0.f, -1.f, 0.f); break;
		case AxisDirection::MinusZ: m_up = float3(0.f, 0.f, -1.f); break;
		}
		
		switch (m_front_dir) {
		case AxisDirection::PlusX:  m_front = float3(1.f, 0.f, 0.f); break;
		case AxisDirection::PlusY:  m_front = float3(0.f, 1.f, 0.f); break;
		case AxisDirection::PlusZ:  m_front = float3(0.f, 0.f, 1.f); break;
		case AxisDirection::MinusX: m_front = float3(-1.f, 0.f, 0.f); break;
		case AxisDirection::MinusY: m_front = float3(0.f, -1.f, 0.f); break;
		case AxisDirection::MinusZ: m_front = float3(0.f, 0.f, -1.f); break;
		}

		if (system == CoordinateSystem::LeftHanded) {
			m_right = cross(m_up, m_front);
			m_left  = -m_right;
		} else {
			m_left  = cross(m_up, m_front);
			m_right = -m_left;
		}
	}

	VERA_NODISCARD VERA_CONSTEXPR CoordinateSystem getCoordinateSystem() const VERA_NOEXCEPT
	{
		return m_coord_system;
	}

	VERA_NODISCARD VERA_CONSTEXPR AxisDirection getUpAxisDirection() const VERA_NOEXCEPT
	{
		return m_up_dir;
	}

	VERA_NODISCARD VERA_CONSTEXPR AxisDirection getFrontAxisDirection() const VERA_NOEXCEPT
	{
		return m_up_dir;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 up() const VERA_NOEXCEPT
	{
		return m_up;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 front() const VERA_NOEXCEPT
	{
		return m_front;
	}

	VERA_NODISCARD VERA_CONSTEXPR float3 left() const VERA_NOEXCEPT
	{
		return m_left;
	}
	
	VERA_NODISCARD VERA_CONSTEXPR float3 right() const VERA_NOEXCEPT
	{
		return m_right;
	}

private:
	CoordinateSystem m_coord_system;
	AxisDirection    m_up_dir;
	AxisDirection    m_front_dir;

	float3           m_up;
	float3           m_front;
	float3           m_left;
	float3           m_right;
};

VERA_NAMESPACE_END
