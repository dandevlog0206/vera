#pragma once

#include "shader_variable.h"
#include <vector>
#include <string_view>

VERA_NAMESPACE_BEGIN

class Device;
class ShaderReflection;
class ShaderStorage;
class PipelineLayout;
class CommandBuffer;

class ShaderParameter
{
public:
	ShaderParameter(obj<ShaderReflection> reflection);
	~ShaderParameter();

	ShaderVariable operator[](std::string_view name);

	obj<Device> getDevice();
	obj<ShaderReflection> getShaderReflection();
	obj<ShaderStorage> getShaderStorage();

private:
	obj<ShaderReflection> m_reflection;
	obj<ShaderStorage>    m_storage;
};

VERA_NAMESPACE_END