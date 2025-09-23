#pragma once

#include "shader_variable.h"
#include <vector>
#include <string_view>

VERA_NAMESPACE_BEGIN

class ShaderReflection;
class PipelineLayout;
class RenderCommand;
struct ShaderStorage;

class ShaderParameter
{
public:
	ShaderParameter(ref<ShaderReflection> reflection);
	~ShaderParameter();

	ShaderVariable operator[](std::string_view name);

	ref<ShaderReflection> getShaderReflection();

	void bindRenderCommand(ref<PipelineLayout> pipeline, ref<RenderCommand> cmd) const;

	bool empty() const;

private:
	ref<ShaderReflection>       m_reflection;
	std::vector<ShaderStorage*> m_storages;
};

VERA_NAMESPACE_END