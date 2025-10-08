#pragma once

#include "object_impl.h"
#include "../../include/vera/core/resource_binding.h"
#include "../../include/vera/util/index_map.h"
#include "../../include/vera/util/hash.h"

VERA_NAMESPACE_BEGIN

enum class ResourceType VERA_ENUM;

struct ResourceBindingDesc
{
	ResourceBindingInfo bindingInfo;
	hash_t              hashValue;
};

struct ResourceArrayBindingDesc
{
	using BindingDescs = std::vector<ResourceBindingDesc>;

	BindingDescs bindingDescs;
	uint32_t     binding;
	ResourceType resourceType;
};

struct ResourceBindingImpl
{
	// BindingMap[binding][array_element] -> resource
	using BindingStates = std::vector<ResourceArrayBindingDesc>;

	ref<Device>               device;
	ref<ResourceBindingPool>  resourceBindingPool;
	const_ref<ResourceLayout> resourceLayout;

	vk::DescriptorSet         descriptorSet;

	hash_t                    hashValue;
	BindingStates             bindingStates;
	uint32_t                  arrayElementCount; // for last binding
	bool                      isCached;
};

VERA_NAMESPACE_END
