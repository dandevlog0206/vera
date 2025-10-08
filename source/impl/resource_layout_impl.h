#pragma once

#include "object_impl.h"

#include "../../include/vera/core/resource_layout.h"

VERA_NAMESPACE_BEGIN

struct ResourceLayoutImpl
{
	using LayoutBindings = std::vector<ResourceLayoutBinding>;

	obj<Device>             device;

	vk::DescriptorSetLayout descriptorSetLayout;

	hash_t                  hashValue;
	LayoutBindings          bindings;
};

static vk::DescriptorSetLayoutCreateFlags to_vk_descriptor_set_layout_create_flags(ResourceLayoutCreateFlags flags)
{
	vk::DescriptorSetLayoutCreateFlags result = {};

	if (flags & ResourceLayoutCreateFlagBits::UpdateAfterBindPool)
		result |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
	if (flags & ResourceLayoutCreateFlagBits::PushBinding)
		result |= vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;
	
	return result;
}

static vk::DescriptorBindingFlags to_vk_descriptor_binding_flags(ResourceLayoutBindingFlags flags)
{
	vk::DescriptorBindingFlags result = {};

	if (flags & ResourceLayoutBindingFlagBits::UpdateAfterBind)
		result |= vk::DescriptorBindingFlagBits::eUpdateAfterBind;
	if (flags & ResourceLayoutBindingFlagBits::UpdateUnusedWhilePending)
		result |= vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
	if (flags & ResourceLayoutBindingFlagBits::PartiallyBound)
		result |= vk::DescriptorBindingFlagBits::ePartiallyBound;
	if (flags & ResourceLayoutBindingFlagBits::VariableBindingCount)
		result |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
	
	return result;
}

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