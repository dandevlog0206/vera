#pragma once

#include "object_impl.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/util/index_map.h"
#include "../../include/vera/util/hash.h"

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;

struct DescriptorBindingDesc
{
	DescriptorBindingInfo bindingInfo;
	hash_t                hashValue;
};

struct DescriptorArrayBindingDesc
{
	using BindingDescs = std::vector<DescriptorBindingDesc>;

	BindingDescs   bindingDescs;
	uint32_t       binding;
	DescriptorType resourceType;
};

struct DescriptorSetImpl
{
	// BindingMap[binding][array_element] -> resource
	using BindingStates = std::vector<DescriptorArrayBindingDesc>;

	ref<Device>                    device;
	ref<DescriptorPool>            descriptorPool;
	const_ref<DescriptorSetLayout> descriptorSetLayout;

	vk::DescriptorSet              descriptorSet;

	hash_t                         hashValue;
	BindingStates                  bindingStates;
	uint32_t                       arrayElementCount; // for last binding
	bool                           isCached;
};

VERA_NAMESPACE_END
