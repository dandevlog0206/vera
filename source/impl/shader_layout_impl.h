#pragma once

#include "object_impl.h"
#include "../../include/vera/core/shader_layout.h"

VERA_NAMESPACE_BEGIN

class ShaderParserInfo;

class ShaderLayoutImpl
{
public:
	obj<Device>                      device            = {};

	std::vector<ShaderLayoutBinding> bindings          = {};
	PushConstantRange                pushConstantRange = {};
	hash_t                           hashValue         = {};
};

VERA_NAMESPACE_END