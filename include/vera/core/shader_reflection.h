#pragma once

#include "shader.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class Pipeline;

class ShaderReflection : protected CoreObject
{
	VERA_CORE_OBJECT(ShaderReflection)
public:
	static ref<ShaderReflection> create(std::vector<ref<Shader>> shaders);
	~ShaderReflection();

	bool isCompatible(ref<Pipeline>& pipeline) const;
};

VERA_NAMESPACE_END