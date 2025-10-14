#pragma once

#include "../core/shader_storage.h"
#include <vector>
#include <string_view>

VERA_NAMESPACE_BEGIN

class Device;
class ShaderStorage;
class PipelineLayout;

class ShaderParameter
{
	friend class CommandBuffer;
public:
	ShaderParameter();
	ShaderParameter(obj<PipelineLayout> layout);
	~ShaderParameter();

	void init(obj<PipelineLayout> layout);

	ShaderVariable operator[](std::string_view name);

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;
	VERA_NODISCARD obj<PipelineLayout> getPipelineLayout() VERA_NOEXCEPT;
	VERA_NODISCARD obj<ShaderStorage> getShaderStorage() VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	obj<PipelineLayout> m_pipeline_layout;
	obj<ShaderStorage>  m_storage;
};

VERA_NAMESPACE_END