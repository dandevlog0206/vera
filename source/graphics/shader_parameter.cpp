#include "../../include/vera/graphics/shader_parameter.h"
#include "../impl/shader_storage_impl.h"

#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/core/shader_storage.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/buffer.h"

VERA_NAMESPACE_BEGIN

ShaderParameter::ShaderParameter(obj<ShaderReflection> reflection) :
	m_reflection(reflection),
	m_storage(ShaderStorage::create(std::move(reflection))) {}

ShaderParameter::~ShaderParameter()
{
}

ShaderVariable ShaderParameter::operator[](std::string_view name)
{
	return m_storage->accessVariable(name);
}

obj<Device> ShaderParameter::getDevice() VERA_NOEXCEPT
{
	return m_reflection->getDevice();
}

obj<ShaderReflection> ShaderParameter::getShaderReflection() VERA_NOEXCEPT
{
	return m_reflection;
}

obj<ShaderStorage> ShaderParameter::getShaderStorage() VERA_NOEXCEPT
{
	return m_storage;
}

bool ShaderParameter::empty() const VERA_NOEXCEPT
{
	auto& impl = CoreObject::getImpl(m_storage);

	return impl.storageDatas.empty();
}

VERA_NAMESPACE_END