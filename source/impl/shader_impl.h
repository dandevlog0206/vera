#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class ShaderParserInfo;

class ShaderImpl
{
public:
	using ShaderParseInfoUnique = std::unique_ptr<ShaderParserInfo>;

	obj<Device>                   device           = {};
	mutable obj<ShaderLayout>     shaderLayout     = {};
	mutable obj<ShaderReflection> shaderReflection = {};

	vk::ShaderModule              shaderModule     = {};

	mutable ShaderParseInfoUnique parserInfo       = {};
	size_t                        hashValue        = {};
};

VERA_NAMESPACE_END
