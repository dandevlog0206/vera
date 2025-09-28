#pragma once

#include "core_object.h"
#include "../shader/shader_variable.h"

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

	ShaderVariable accessVariable(std::string_view name);

	uint32_t getFrameCount();

	void bindCommandBuffer(ref<PipelineLayout> layout, ref<CommandBuffer> cmd) const;
};

VERA_NAMESPACE_END