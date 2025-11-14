#pragma once

#include "../../include/vera/core/enum_types.h"
#include "../../include/vera/util/array_view.h"
#include <memory_resource>

VERA_NAMESPACE_BEGIN

class SpvParser
{
public:
	array_view<uint32_t> strip_reflection_info(const uint32_t* spirv_code, size_t size_in_byte);

private:
	std::pmr::monotonic_buffer_resource m_memory;
};

VERA_NAMESPACE_END