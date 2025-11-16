#pragma once

#include "../core/coredefs.h"
#include <string>
#include <bit>

VERA_NAMESPACE_BEGIN

class Version
{
public:
	Version() = default;

	Version(uint32_t version) :
		major(version >> 22), minor(minor >> 12), patch(patch) {}
	
	Version(uint32_t major, uint32_t minor, uint32_t patch) :
		major(major), minor(minor), patch(patch) {}

	std::string format() const
	{
		std::string str;
		str += std::to_string(major) + '.';
		str += std::to_string(minor) + '.';
		str += std::to_string(patch);

		return str;
	}

	operator uint32_t() const
	{
		return std::bit_cast<uint32_t>(*this);
	}

	uint32_t major : 10;
	uint32_t minor : 10;
	uint32_t patch : 12;
};

VERA_NAMESPACE_END
