#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class ProgramReflectionImpl
{
public:
	obj<Device>                        device;
	std::vector<obj<ShaderReflection>> shaderReflections;

	hash_t                             hashValue;
};

VERA_NAMESPACE_END
