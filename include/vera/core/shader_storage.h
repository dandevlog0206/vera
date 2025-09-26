#pragma once

#include "core_object.h"

VERA_NAMESPACE_BEGIN

class ShaderReflection;

class ShaderStorage : protected CoreObject
{
	VERA_CORE_OBJECT(ShaderStorage);
public:
	static ref<ShaderStorage> create(ref<ShaderReflection> reflection);
	~ShaderStorage();
};

VERA_NAMESPACE_END