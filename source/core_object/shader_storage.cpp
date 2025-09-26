#include "../../include/vera/core/shader_storage.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/device.h"
#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"

VERA_NAMESPACE_BEGIN

static ShaderStorageData* create_resource_storage(const ReflectionResourceDesc& desc)
{
	switch (desc.resourceType) {
	case ResourceType::Sampler: {
		auto* result = new SamplerStorage;
		result->storageType      = ShaderStorageDataType::Sampler;
		result->resourceType     = ResourceType::Sampler;
		result->shaderStageFlags = desc.shaderStageFlags;
		return result;
	}
	case ResourceType::CombinedImageSampler: {
		auto* result = new CombinedImageSamplerStorage;
		result->storageType      = ShaderStorageDataType::CombinedImageSampler;
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

static ShaderStorageData* create_resource_block_storage(const ReflectionResourceBlockDesc& desc)
{
	switch (desc.resourceType) {
	case ResourceType::UniformBuffer:
	case ResourceType::StorageBuffer: 
	case ResourceType::UniformBufferDynamic:
	case ResourceType::StorageBufferDynamic: {
		auto* result = new BufferBlockStorage;
		result->storageType      = ShaderStorageDataType::BufferBlock;
		result->resourceType     = desc.resourceType;
		result->shaderStageFlags = desc.shaderStageFlags;
		result->blockStorage.resize(desc.sizeInByte);
		return result;
	}
	}

	throw Exception("failed to create resource block storage");
}

static ShaderStorageData* create_push_constant_storage(const ReflectionPushConstantDesc& desc)
{
	auto* result = new PushConstantStorage;
	result->storageType      = ShaderStorageDataType::PushConstant;
	result->resourceType     = ResourceType::Unknown;
	result->shaderStageFlags = desc.shaderStageFlags;
	result->blockStorage.resize(desc.sizeInByte);
	return result;
}

static ShaderStorageData* create_resource_array_storage(const ReflectionResourceArrayDesc& desc)
{
	auto* result = new ResourceArrayStorage;
	result->storageType      = ShaderStorageDataType::ResourceArray;
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

static void destroy_storage(ShaderStorageData* ptr)
{
	if (ptr->storageType == ShaderStorageDataType::ResourceArray) {
		auto& storage = *static_cast<ResourceArrayStorage*>(ptr);
		
		for (uint32_t i = 0; i < storage.elementCount; ++i)
			destroy_storage(storage.elements[i]);
	}

	delete ptr;
}

ref<ShaderStorage> ShaderStorage::create(ref<ShaderReflection> reflection)
{
	auto  obj       = createNewObject<ShaderStorage>();
	auto& impl      = getImpl(obj);
	auto& refl_impl = getImpl(reflection);
	auto  vk_device = get_vk_device(refl_impl.device);

	impl.device = refl_impl.device;

	vk::DescriptorPoolSize pool_sizes[] = {
		{ vk::DescriptorType::eSampler, 100 },
		{ vk::DescriptorType::eCombinedImageSampler, 100 },
	};

	vk::DescriptorPoolCreateInfo pool_info;
	pool_info.maxSets       = 3; // TODO: figure out vk::DescriptorPoolCreateInfo::maxSets
	pool_info.poolSizeCount = static_cast<uint32_t>(VERA_LENGTHOF(pool_sizes));
	pool_info.pPoolSizes    = pool_sizes;

	impl.descriptorPool = vk_device.createDescriptorPool(pool_info);

	impl.storages.reserve(refl_impl.descriptors.size());
	for (auto* desc_ptr : refl_impl.descriptors) {
		switch (desc_ptr->type) {
		case ReflectionType::Resource: {
			auto& desc = *static_cast<ReflectionResourceDesc*>(desc_ptr);
			impl.storages.push_back(create_resource_storage(desc));
			
			// TODO: remove later!!!!!!!!!!!!!!!
			if (desc.resourceType == ResourceType::CombinedImageSampler) {
				auto& storage = *static_cast<CombinedImageSamplerStorage*>(impl.storages.back());

				storage.sampler = Sampler::create(impl.device);

				vk::DescriptorSetAllocateInfo alloc_info;
				alloc_info.descriptorPool     = impl.descriptorPool;
				alloc_info.descriptorSetCount = 1;
				alloc_info.pSetLayouts        = &get_descriptor_set_layout(desc_ptr->resourceLayout);

				vk_device.allocateDescriptorSets(&alloc_info, &storage.descriptorSet);
			}
		} break;
		case ReflectionType::ResourceBlock: {
			auto& desc = *static_cast<ReflectionResourceBlockDesc*>(desc_ptr);
			impl.storages.push_back(create_resource_block_storage(desc));
		} break;
		case ReflectionType::PushConstant: {
			auto& desc = *static_cast<ReflectionPushConstantDesc*>(desc_ptr);
			impl.storages.push_back(create_push_constant_storage(desc));
		} break;
		case ReflectionType::ResourceArray: {
			auto& desc = *static_cast<ReflectionResourceArrayDesc*>(desc_ptr);
			impl.storages.push_back(create_resource_array_storage(desc));
		} break;
		default:
			throw Exception("invalid reflection");
		}
	}

	return obj;
}

ShaderStorage::~ShaderStorage()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	for (auto* storage : impl.storages)
		destroy_storage(storage);

	vk_device.destroy(impl.descriptorPool);

	destroyObjectImpl(this);
}

VERA_NAMESPACE_END