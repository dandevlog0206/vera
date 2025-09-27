#pragma once

#include "shader_variable.h"
#include <vector>
#include <string_view>

VERA_NAMESPACE_BEGIN

class ShaderReflection;
class ShaderStorage;
class PipelineLayout;
class RenderCommand;

class ShaderParameter
{
public:
	ShaderParameter(obj<ShaderReflection> reflection);
	~ShaderParameter();

	ShaderVariable operator[](std::string_view name);

	obj<ShaderReflection> getShaderReflection();

	void bindRenderCommand(obj<PipelineLayout> pipeline, obj<RenderCommand> cmd) const;

	bool empty() const;

private:
	obj<ShaderReflection> m_reflection;
	obj<ShaderStorage>    m_storage;
};

VERA_NAMESPACE_END