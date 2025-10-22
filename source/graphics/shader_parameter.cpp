#include "../../include/vera/graphics/shader_parameter.h"
#include "../impl/shader_storage_impl.h"

#include "../../include/vera/core/shader_storage.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/buffer.h"

VERA_NAMESPACE_BEGIN

ShaderParameter::ShaderParameter()
{
}

ShaderParameter::ShaderParameter(obj<PipelineLayout> layout)
{
	init(layout);
}

ShaderParameter::~ShaderParameter()
{
}

void ShaderParameter::init(obj<PipelineLayout> layout)
{
	if (!layout->hasReflection()) return;

	m_pipeline_layout = std::move(layout);
	m_storage         = ShaderStorage::create(m_pipeline_layout);
}

ShaderVariable ShaderParameter::operator[](std::string_view name)
{
	return m_storage->accessVariable(name);
}

obj<Device> ShaderParameter::getDevice() VERA_NOEXCEPT
{
	return m_pipeline_layout->getDevice();
}

obj<PipelineLayout> ShaderParameter::getPipelineLayout() VERA_NOEXCEPT
{
	return m_pipeline_layout;
}

obj<ShaderStorage> ShaderParameter::getShaderStorage() VERA_NOEXCEPT
{
	return m_storage;
}

bool ShaderParameter::hasVariable(std::string_view name) VERA_NOEXCEPT
{
	return m_storage && m_storage->hasVariable(name);
}

bool ShaderParameter::empty() const VERA_NOEXCEPT
{
	return !m_storage;
}

VERA_NAMESPACE_END