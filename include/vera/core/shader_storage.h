#pragma once

#include "core_object.h"
#include "../shader/shader_variable.h"

VERA_NAMESPACE_BEGIN

class Device;
class ShaderReflection;
class CommandBuffer;
class PipelineLayout;

struct ShaderStorageAddress
{
	uint32_t resourceIndex = 0;
	uint32_t arrayIndex    = 0;
	uint32_t blockOffset   = 0;
};

class ShaderStorage : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderStorage)
public:
	static obj<ShaderStorage> create(obj<ShaderReflection> reflection);
	~ShaderStorage();

	obj<Device> getDevice();
	obj<ShaderReflection> getShaderReflection();

	ShaderVariable accessVariable(std::string_view name);
};

VERA_NAMESPACE_END