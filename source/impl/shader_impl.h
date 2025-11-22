#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class ShaderParserInfo;

class ShaderImpl
{
public:
	using ShaderParseInfoUnique = std::unique_ptr<ShaderParserInfo>;

	obj<Device>           device           = {};
	obj<ShaderReflection> shaderReflection = {};

	vk::ShaderModule      vkShaderModule   = {};

	std::vector<uint32_t> spirvCode        = {};
	std::string_view      entryPointName   = {};
	ShaderStageFlags      stageFlags       = {};
	size_t                hashValue        = {};
};

VERA_NAMESPACE_END
