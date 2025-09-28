#include "../../include/vera/shader/shader_parameter.h"
#include "../impl/shader_reflection_impl.h"
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
	auto& refl_impl    = CoreObject::getImpl(m_reflection);
	auto& storage_impl = CoreObject::getImpl(m_storage);
	auto& frame        = storage_impl.frames[storage_impl.frameIndex];

	if (auto iter = refl_impl.hashMap.find(name); iter != refl_impl.hashMap.end()) {
		auto* storage_ptr = frame.storages[iter->second];
		auto* desc_ptr    = refl_impl.descriptors[iter->second];

		return ShaderVariable(storage_ptr, desc_ptr, UINT32_MAX);
	}

	throw Exception("couldn't find resource named " + std::string(name));
}

obj<Device> ShaderParameter::getDevice()
{
	return m_reflection->getDevice();
}

obj<ShaderReflection> ShaderParameter::getShaderReflection()
{
	return m_reflection;
}

obj<ShaderStorage> ShaderParameter::getShaderStorage()
{
	return m_storage;
}

void ShaderParameter::bindCommandBuffer(ref<PipelineLayout> layout, ref<CommandBuffer> cmd) const
{
	m_storage->bindCommandBuffer(layout, cmd);
}

VERA_NAMESPACE_END