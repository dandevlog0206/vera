#pragma once

#include "object_impl.h"
#include <unordered_map>

VERA_NAMESPACE_BEGIN

class DescriptorSetImpl
{
public:
	using BindingStateMap = std::unordered_map<uint64_t, obj<CoreObject>> ;

	obj<Device>              device                  = {};
	obj<DescriptorPool>      descriptorPool          = {};
	obj<DescriptorSetLayout> descriptorSetLayout     = {};

	vk::DescriptorSet        vkDescriptorSet         = {};

	BindingStateMap          bindingStates           = {};
	uint32_t                 variableDescriptorCount = {};
};

VERA_NAMESPACE_END
