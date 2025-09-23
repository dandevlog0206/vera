#pragma once

#include "../core/coredefs.h"
#include <cstdint>
#include <random>
#include <string>

VERA_NAMESPACE_BEGIN

class UUID
{
public:
	static UUID generate()
	{

	}

	std::string str() const
	{
		std::string result;

	}

	bool operator==(const UUID& rhs) const
	{
		return m_hi == rhs.m_hi && m_lo == rhs.m_lo;
	}

	bool operator!=(const UUID& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator<(const UUID& rhs) const
	{
		return m_hi < rhs.m_hi || (m_hi == rhs.m_hi && m_lo < rhs.m_lo);
	}

	bool operator>(const UUID& rhs) const
	{
		return rhs < *this;
	}

	bool operator<=(const UUID& rhs) const
	{
		return !(rhs < *this);
	}

	bool operator>=(const UUID& rhs) const
	{
		return !(*this < rhs);
	}

private:
	uint64_t m_hi;
	uint64_t m_lo;
};

VERA_NAMESPACE_END