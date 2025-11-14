#pragma once

#include "../core/coredefs.h"

VERA_NAMESPACE_BEGIN

class bool32
{
public:
	VERA_CONSTEXPR bool32() VERA_NOEXCEPT :
		m_value(0) {}

	VERA_CONSTEXPR bool32(const bool32& rhs) VERA_NOEXCEPT :
		m_value(rhs.m_value) {}

	VERA_CONSTEXPR bool32(bool value) VERA_NOEXCEPT :
		m_value(value ? 1 : 0) {}

	VERA_CONSTEXPR bool32& operator=(const bool32& rhs) VERA_NOEXCEPT
	{
		m_value = rhs.m_value;
		return *this;
	}

	VERA_CONSTEXPR bool32& operator=(bool value) VERA_NOEXCEPT
	{
		m_value = value ? 1 : 0;
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR operator bool() const VERA_NOEXCEPT
	{
		return m_value != 0;
	}

private:
	uint32_t m_value;
};

VERA_NAMESPACE_END
