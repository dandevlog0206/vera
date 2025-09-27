#pragma once

#include "object_impl.h"

#include "../../include/vera/core/resource_layout.h"

VERA_NAMESPACE_BEGIN

struct ResourceLayoutImpl
{
	obj<Device>                        device;

	vk::DescriptorSetLayout            layout;

	size_t                             hashValue;
	std::vector<ResourceLayoutBinding> bindings;
};

static vk::DescriptorType to_vk_descriptor_type(ResourceType type)
{
	switch (type) {
	case ResourceType::Sampler:              return vk::DescriptorType::eSampler;
	case ResourceType::CombinedImageSampler: return vk::DescriptorType::eCombinedImageSampler;
	case ResourceType::SampledImage:         return vk::DescriptorType::eSampledImage;
	case ResourceType::StorageImage:         return vk::DescriptorType::eStorageImage;
	case ResourceType::UniformTexelBuffer:   return vk::DescriptorType::eUniformTexelBuffer;
	case ResourceType::StorageTexelBuffer:   return vk::DescriptorType::eStorageTexelBuffer;
	case ResourceType::UniformBuffer:        return vk::DescriptorType::eUniformBuffer;
	case ResourceType::StorageBuffer:        return vk::DescriptorType::eStorageBuffer;
	case ResourceType::UniformBufferDynamic: return vk::DescriptorType::eUniformBufferDynamic;
	case ResourceType::StorageBufferDynamic: return vk::DescriptorType::eStorageBufferDynamic;
	case ResourceType::InputAttachment:      return vk::DescriptorType::eInputAttachment;
	}

	VERA_ASSERT_MSG(false, "invalid resource type");
	return {};
}

VERA_NAMESPACE_END