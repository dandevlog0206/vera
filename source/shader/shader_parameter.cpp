#include "../../include/vera/shader/shader_parameter.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/pipeline_layout_impl.h"

#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/core/shader_storage.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/render_command.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/buffer.h"

VERA_NAMESPACE_BEGIN

ShaderParameter::ShaderParameter(ref<ShaderReflection> reflection) :
	m_reflection(reflection),
	m_storage(ShaderStorage::create(std::move(reflection))) {}

ShaderParameter::~ShaderParameter()
{

}

ShaderVariable ShaderParameter::operator[](std::string_view name)
{
	auto& refl_impl    = CoreObject::getImpl(m_reflection);
	auto& storage_impl = CoreObject::getImpl(m_storage);

	if (auto iter = refl_impl.hashMap.find(name); iter != refl_impl.hashMap.end())
		return ShaderVariable(storage_impl.storages[iter->second], refl_impl.descriptors[iter->second], UINT32_MAX);

	throw Exception("couldn't find resource named " + std::string(name));
}

ref<ShaderReflection> ShaderParameter::getShaderReflection()
{
	return m_reflection;
}

void ShaderParameter::bindRenderCommand(ref<PipelineLayout> layout, ref<RenderCommand> cmd) const
{
	auto& storage_impl = CoreObject::getImpl(m_storage);
	auto& layout_impl  = CoreObject::getImpl(layout);
	auto  vk_cmd       = get_vk_command_buffer(cmd);

	for (auto& storage : storage_impl.storages) {
		switch (storage->storageType) {
		case ShaderStorageDataType::ResourceArray: {
		} break;
		case ShaderStorageDataType::Sampler: {
		} break;
		case ShaderStorageDataType::Texture: {
		} break;
		case ShaderStorageDataType::CombinedImageSampler: {
			auto& sampler = *static_cast<CombinedImageSamplerStorage*>(storage);

			vk_cmd.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				layout_impl.layout,
				0,
				sampler.descriptorSet,
				{});
		} break;
		case ShaderStorageDataType::Buffer: {
		} break;
		case ShaderStorageDataType::BufferBlock: {
		} break;
		case ShaderStorageDataType::PushConstant: {
			auto& pc = *static_cast<PushConstantStorage*>(storage);

			vk_cmd.pushConstants(
				layout_impl.layout,
				to_vk_shader_stage_flags(pc.shaderStageFlags),
				0,
				static_cast<uint32_t>(pc.blockStorage.size()),
				pc.blockStorage.data());
		} break;
		}
	}
}

bool ShaderParameter::empty() const
{
	return CoreObject::getImpl(m_storage).storages.empty();
}

VERA_NAMESPACE_END