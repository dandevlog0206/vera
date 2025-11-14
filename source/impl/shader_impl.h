#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

struct PushConstantRange;

class ShaderImpl
{
public:
	obj<Device>           device           = {};
	obj<ShaderReflection> shaderReflection = {};

	vk::ShaderModule      shader           = {};

	std::vector<uint32_t> spirvCode        = {};
	size_t                hashValue        = {};

	obj<ShaderReflection> getOrCreateShaderReflection() VERA_NOEXCEPT;
};

VERA_NAMESPACE_END
