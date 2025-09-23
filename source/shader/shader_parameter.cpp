#include "../../include/vera/shader/shader_parameter.h"
#include "shader_storage.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/pipeline_layout_impl.h"
#include "../impl/render_command_impl.h"

#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/render_command.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/buffer.h"

VERA_NAMESPACE_BEGIN

static ShaderStorage* create_resource_storage(const ReflectionResourceDesc& desc)
{
	switch (desc.resourceType) {
	case ResourceType::Sampler: {
		auto*result = new SamplerStorage;
		result->storageType      = ShaderStorageType::Sampler;
		result->resourceType     = ResourceType::Sampler;
		result->shaderStageFlags = desc.shaderStageFlags;
		return result;
	}
	case ResourceType::CombinedImageSampler: {
		auto*result = new CombinedImageSamplerStorage;
		result->storageType      = ShaderStorageType::CombinedImageSampler;
		result->resourceType     = ResourceType::CombinedImageSampler;
		result->shaderStageFlags = desc.shaderStageFlags;
		return result;
	}
	case ResourceType::SampledImage: {
	}
	case ResourceType::StorageImage: {
	}
	case ResourceType::UniformTexelBuffer: {
	}
	case ResourceType::StorageTexelBuffer: {
	}
	}

	throw Exception("failed to create resource storage");
}

static ShaderStorage* create_resource_block_storage(const ReflectionResourceBlockDesc& desc)
{
	switch (desc.resourceType) {
	case ResourceType::UniformBuffer:
	case ResourceType::StorageBuffer: 
	case ResourceType::UniformBufferDynamic:
	case ResourceType::StorageBufferDynamic: {
		auto* result = new BufferBlockStorage;
		result->storageType      = ShaderStorageType::BufferBlock;
		result->resourceType     = desc.resourceType;
		result->shaderStageFlags = desc.shaderStageFlags;
		result->blockStorage.resize(desc.sizeInByte);
		return result;
	}
	}

	throw Exception("failed to create resource block storage");
}

static ShaderStorage* create_push_constant_storage(const ReflectionPushConstantDesc& desc)
{
	auto* result = new PushConstantStorage;
	result->storageType      = ShaderStorageType::PushConstant;
	result->resourceType     = ResourceType::Unknown;
	result->shaderStageFlags = desc.shaderStageFlags;
	result->blockStorage.resize(desc.sizeInByte);
	return result;
}

static ShaderStorage* create_resource_array_storage(const ReflectionResourceArrayDesc& desc)
{
	auto* result = new ResourceArrayStorage;
	result->storageType      = ShaderStorageType::ResourceArray;
	result->resourceType     = desc.resourceType;
	result->shaderStageFlags = desc.shaderStageFlags;
	result->elementCount     = desc.elementCount;
	result->elements.reserve(desc.elementCount);

	if (desc.element->type == ReflectionType::Resource) {
		for (uint32_t i = 0; i < desc.elementCount; ++i) {
			auto& resource_desc = *static_cast<ReflectionResourceDesc*>(desc.element);
			result->elements.push_back(create_resource_storage(resource_desc));
		}
		return result;
	} else if (desc.element->type == ReflectionType::ResourceBlock) {
		for (uint32_t i = 0; i < desc.elementCount; ++i) {
			auto& block_desc = *static_cast<ReflectionResourceBlockDesc*>(desc.element);
			result->elements.push_back(create_resource_block_storage(block_desc));
		}
		return result;
	}

	throw Exception("failed to create resource array storage");
}

static void destroy_storage(ShaderStorage* ptr)
{
	if (ptr->storageType == ShaderStorageType::ResourceArray) {
		auto& storage = *static_cast<ResourceArrayStorage*>(ptr);
		
		for (uint32_t i = 0; i < storage.elementCount; ++i)
			destroy_storage(storage.elements[i]);
	}

	delete ptr;
}

ShaderParameter::ShaderParameter(ref<ShaderReflection> reflection) :
	m_reflection(std::move(reflection))
{
	auto& impl = CoreObject::getImpl(m_reflection);

	m_storages.reserve(impl.descriptors.size());
	for (auto* desc_ptr : impl.descriptors) {
		switch (desc_ptr->type) {
		case ReflectionType::Resource: {
			auto& desc = *static_cast<ReflectionResourceDesc*>(desc_ptr);
			m_storages.push_back(create_resource_storage(desc));
		} break;
		case ReflectionType::ResourceBlock: {
			auto& desc = *static_cast<ReflectionResourceBlockDesc*>(desc_ptr);
			m_storages.push_back(create_resource_block_storage(desc));
		} break;
		case ReflectionType::PushConstant: {
			auto& desc = *static_cast<ReflectionPushConstantDesc*>(desc_ptr);
			m_storages.push_back(create_push_constant_storage(desc));
		} break;
		case ReflectionType::ResourceArray: {
			auto& desc = *static_cast<ReflectionResourceArrayDesc*>(desc_ptr);
			m_storages.push_back(create_resource_array_storage(desc));
		} break;
		default:
			throw Exception("invalid reflection");
		}
	}
}

ShaderParameter::~ShaderParameter()
{
	for (auto* storage : m_storages)
		destroy_storage(storage);
}

ShaderVariable ShaderParameter::operator[](std::string_view name)
{
	auto& impl = CoreObject::getImpl(m_reflection);

	if (auto iter = impl.hashMap.find(name); iter != impl.hashMap.end())
		return ShaderVariable(m_storages[iter->second], impl.descriptors[iter->second], UINT32_MAX);

	throw Exception("couldn't find resource named " + std::string(name));
}

ref<ShaderReflection> ShaderParameter::getShaderReflection()
{
	return m_reflection;
}

void ShaderParameter::bindRenderCommand(ref<PipelineLayout> layout, ref<RenderCommand> cmd) const
{
	auto& impl        = CoreObject::getImpl(cmd);
	auto& layout_impl = CoreObject::getImpl(layout);

	for (auto& storage : m_storages) {
		switch (storage->storageType) {
		case ShaderStorageType::PushConstant:
			auto& pc = *static_cast<PushConstantStorage*>(storage);

			impl.commandBuffer.pushConstants(
				layout_impl.layout,
				to_vk_shader_stage_flags(pc.shaderStageFlags),
				0,
				static_cast<uint32_t>(pc.blockStorage.size()),
				pc.blockStorage.data());
		break;
		}
	}
}

bool ShaderParameter::empty() const
{
	return m_storages.empty();
}

VERA_NAMESPACE_END