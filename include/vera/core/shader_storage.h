#pragma once

#include "core_object.h"

VERA_NAMESPACE_BEGIN

class Device;
class ShaderReflection;
class CommandBuffer;
class PipelineLayout;

class ShaderStorage : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderStorage)
public:
	static obj<ShaderStorage> create(obj<ShaderReflection> reflection);
	~ShaderStorage();

	obj<Device> getDevice();
	obj<ShaderReflection> getShaderReflection();

	uint32_t getFrameCount();

	void bindCommandBuffer(ref<PipelineLayout> layout, ref<CommandBuffer> cmd) const;
};

VERA_NAMESPACE_END