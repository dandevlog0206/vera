#pragma once

#include "object_impl.h"

#include "../../include/vera/core/descriptor_set_layout.h"

VERA_NAMESPACE_BEGIN

struct DescriptorSetLayoutImpl
{
	using LayoutBindings = std::vector<DescriptorSetLayoutBinding>;

	obj<Device>              device;

	vk::DescriptorSetLayout  descriptorSetLayout;

	hash_t                   hashValue;
	LayoutBindings           bindings;
};

static vk::DescriptorSetLayoutCreateFlags to_vk_descriptor_set_layout_create_flags(DescriptorSetLayoutCreateFlags flags)
{
	vk::DescriptorSetLayoutCreateFlags result = {};

	if (flags & DescriptorSetLayoutCreateFlagBits::UpdateAfterBindPool)
		result |= vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
	if (flags & DescriptorSetLayoutCreateFlagBits::PushBinding)
		result |= vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;
	
	return result;
}

static vk::DescriptorBindingFlags to_vk_descriptor_binding_flags(DescriptorSetLayoutBindingFlags flags)
{
	vk::DescriptorBindingFlags result = {};

	if (flags & DescriptorSetLayoutBindingFlagBits::UpdateAfterBind)
		result |= vk::DescriptorBindingFlagBits::eUpdateAfterBind;
	if (flags & DescriptorSetLayoutBindingFlagBits::UpdateUnusedWhilePending)
		result |= vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending;
	if (flags & DescriptorSetLayoutBindingFlagBits::PartiallyBound)
		result |= vk::DescriptorBindingFlagBits::ePartiallyBound;
	if (flags & DescriptorSetLayoutBindingFlagBits::VariableBindingCount)
		result |= vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
	
	return result;
}

static vk::DescriptorType to_vk_descriptor_type(DescriptorType type)
{
	switch (type) {
	case DescriptorType::Sampler:              return vk::DescriptorType::eSampler;
	case DescriptorType::CombinedImageSampler: return vk::DescriptorType::eCombinedImageSampler;
	case DescriptorType::SampledImage:         return vk::DescriptorType::eSampledImage;
	case DescriptorType::StorageImage:         return vk::DescriptorType::eStorageImage;
	case DescriptorType::UniformTexelBuffer:   return vk::DescriptorType::eUniformTexelBuffer;
	case DescriptorType::StorageTexelBuffer:   return vk::DescriptorType::eStorageTexelBuffer;
	case DescriptorType::UniformBuffer:        return vk::DescriptorType::eUniformBuffer;
	case DescriptorType::StorageBuffer:        return vk::DescriptorType::eStorageBuffer;
	case DescriptorType::UniformBufferDynamic: return vk::DescriptorType::eUniformBufferDynamic;
	case DescriptorType::StorageBufferDynamic: return vk::DescriptorType::eStorageBufferDynamic;
	case DescriptorType::InputAttachment:      return vk::DescriptorType::eInputAttachment;
	}

	VERA_ASSERT_MSG(false, "invalid descriptor type");
	return {};
}

VERA_NAMESPACE_END