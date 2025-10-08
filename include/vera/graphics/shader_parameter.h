#pragma once

#include "../core/shader_storage.h"
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

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;
	VERA_NODISCARD obj<ShaderReflection> getShaderReflection() VERA_NOEXCEPT;
	VERA_NODISCARD obj<ShaderStorage> getShaderStorage() VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	obj<ShaderReflection> m_reflection;
	obj<ShaderStorage>    m_storage;
};

VERA_NAMESPACE_END