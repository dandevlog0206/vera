#include "../../include/vera/core/shader_storage.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/pipeline_layout_impl.h"

#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
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

// TODO: use later
static uint32_t get_descriptor_max_count()
{

}

static void append_shader_storage_frame(ShaderStorageImpl& impl)
{
	auto& refl_impl = CoreObject::getImpl(impl.reflection);
	auto  vk_device = get_vk_device(impl.device);
	auto& frame     = impl.frames.emplace_back();

	// TODO: findout appropriate pool size
	vk::DescriptorPoolSize pool_sizes[] = {
		{ vk::DescriptorType::eSampler, 100 },
		{ vk::DescriptorType::eCombinedImageSampler, 100 },
	};

	uint32_t max_set_count = 200;

	vk::DescriptorPoolCreateInfo pool_info;
	pool_info.maxSets       = max_set_count;
	pool_info.poolSizeCount = static_cast<uint32_t>(VERA_LENGTHOF(pool_sizes));
	pool_info.pPoolSizes    = pool_sizes;

	frame.descriptorPool = vk_device.createDescriptorPool(pool_info);

	frame.storages.reserve(refl_impl.descriptors.size());
	for (auto* desc_ptr : refl_impl.descriptors) {
		switch (desc_ptr->type) {
		case ReflectionType::Resource: {
			auto& desc = *static_cast<ReflectionResourceDesc*>(desc_ptr);
			frame.storages.push_back(create_resource_storage(desc));
		} break;
		case ReflectionType::ResourceBlock: {
			auto& desc = *static_cast<ReflectionResourceBlockDesc*>(desc_ptr);
			frame.storages.push_back(create_resource_block_storage(desc));
		} break;
		case ReflectionType::PushConstant: {
			auto& desc = *static_cast<ReflectionPushConstantDesc*>(desc_ptr);
			frame.storages.push_back(create_push_constant_storage(desc));
		} break;
		case ReflectionType::ResourceArray: {
			auto& desc = *static_cast<ReflectionResourceArrayDesc*>(desc_ptr);
			frame.storages.push_back(create_resource_array_storage(desc));
		} break;
		default:
			throw Exception("invalid reflection");
		}
	}
}

static void allocate_descriptor_set(ShaderStorageImpl& impl, uint32_t frame_idx)
{
	auto& refl_impl = CoreObject::getImpl(impl.reflection);
	auto  vk_device = get_vk_device(impl.device);
	auto& frame     = impl.frames[frame_idx];

	for (uint32_t i = 0; i < refl_impl.descriptors.size(); ++i) {
		auto* desc_ptr = refl_impl.descriptors[i];

		switch (desc_ptr->type) {
		case ReflectionType::Resource: {
			auto& desc = *static_cast<ReflectionResourceDesc*>(desc_ptr);

			// TODO: remove later!!!!!!!!!!!!!!!
			if (desc.resourceType == ResourceType::CombinedImageSampler) {
				auto& storage = *static_cast<CombinedImageSamplerStorage*>(frame.storages[i]);

				storage.sampler = Sampler::create(impl.device);

				vk::DescriptorSetAllocateInfo alloc_info;
				alloc_info.descriptorPool     = frame.descriptorPool;
				alloc_info.descriptorSetCount = 1;
				alloc_info.pSetLayouts        = &get_descriptor_set_layout(desc_ptr->resourceLayout);

				vk_device.allocateDescriptorSets(&alloc_info, &storage.descriptorSet);
			}
		} break;
		case ReflectionType::ResourceBlock: {
			auto& desc = *static_cast<ReflectionResourceBlockDesc*>(desc_ptr);
		} break;
		case ReflectionType::PushConstant: {
			auto& desc = *static_cast<ReflectionPushConstantDesc*>(desc_ptr);
		} break;
		case ReflectionType::ResourceArray: {
			auto& desc = *static_cast<ReflectionResourceArrayDesc*>(desc_ptr);
		} break;
		default:
			throw Exception("invalid reflection");
		}
	}
}

static void destroy_shader_storage_frame(ShaderStorageImpl& impl)
{
	auto vk_device = get_vk_device(impl.device);

	for (auto& frame : impl.frames) {
		for (auto* storage : frame.storages)
			destroy_storage(storage);

		vk_device.destroy(frame.descriptorPool);
	}
}

obj<ShaderStorage> ShaderStorage::create(obj<ShaderReflection> reflection)
{
	auto  obj       = createNewObject<ShaderStorage>();
	auto& impl      = getImpl(obj);
	auto& refl_impl = getImpl(reflection);
	auto  vk_device = get_vk_device(refl_impl.device);

	impl.device         = refl_impl.device;
	impl.reflection     = std::move(reflection);
	impl.frameIndex     = 0;
	impl.lastFrameIndex = -1;

	if (!refl_impl.descriptors.empty()) {
		append_shader_storage_frame(impl);
		allocate_descriptor_set(impl, 0);
	}

	return obj;
}

ShaderStorage::~ShaderStorage()
{
	auto& impl = getImpl(this);

	destroy_shader_storage_frame(impl);

	destroyObjectImpl(this);
}

obj<Device> ShaderStorage::getDevice()
{
	return getImpl(this).device;
}

obj<ShaderReflection> ShaderStorage::getShaderReflection()
{
	return getImpl(this).reflection;
}

uint32_t ShaderStorage::getFrameCount()
{
	return static_cast<uint32_t>(getImpl(this).frames.size());
}

void ShaderStorage::bindCommandBuffer(ref<PipelineLayout> layout, ref<CommandBuffer> cmd) const
{
	auto& impl = getImpl(this);
	
	if (impl.frames.empty()) return;

	auto& layout_impl = getImpl(layout);
	auto& frame       = impl.frames[impl.frameIndex];
	auto  vk_cmd      = get_vk_command_buffer(cmd);

	for (auto& storage : frame.storages) {
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

VERA_NAMESPACE_END