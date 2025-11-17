#pragma once

#include "object_impl.h"
#include <unordered_map>

VERA_NAMESPACE_BEGIN

class DescriptorPoolImpl
{
public:
	using AllocatedMap         = std::unordered_map<hash_t, ref<DescriptorSet>>;
	using DescriptorSetPool    = std::vector<obj<DescriptorSet>>;
	using DescriptorSetPoolMap = std::unordered_map<hash_t, DescriptorSetPool>;

	obj<Device>               device           = {};

	vk::DescriptorPool        vkDescriptorPool = {};

	AllocatedMap              allocatedSets    = {};
	DescriptorSetPoolMap      poolMap          = {};
	DescriptorPoolCreateFlags flags            = {};
};

VERA_NAMESPACE_END
