#pragma once

#include "core_object.h"

VERA_NAMESPACE_BEGIN

class Device;
class ShaderReflection;

class ShaderStorage : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderStorage)
public:
	static obj<ShaderStorage> create(obj<ShaderReflection> reflection);
	~ShaderStorage();

	obj<Device> getDevice();
	obj<ShaderReflection> getShaderReflection();
};

VERA_NAMESPACE_END