#pragma once

#include "shader.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class Pipeline;

class ShaderReflection : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderReflection)
public:
	static obj<ShaderReflection> create(std::vector<obj<Shader>> shaders);
	~ShaderReflection();

	obj<Device> getDevice();

	bool isCompatible(obj<Pipeline>& pipeline) const;
};

VERA_NAMESPACE_END