#pragma once

#include "format.h"
#include <cstdint>

VERA_NAMESPACE_BEGIN

uint32_t get_format_size(Format format);
uint32_t get_format_component_count(Format format);
bool format_has_depth(Format format);
bool format_has_stencil(Format format);
bool format_is_depth_stencil(Format format);
bool format_has_alpha(Format format);

VERA_NAMESPACE_END