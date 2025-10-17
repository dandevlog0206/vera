#pragma once

#include "object_impl.h"
#include <unordered_map>

VERA_NAMESPACE_BEGIN

struct DescriptorPoolImpl
{
	using DescriptorSetMap = std::unordered_map<uint64_t, ref<DescriptorSet>>;

	obj<Device>        device;

	vk::DescriptorPool descriptorPool;

	DescriptorSetMap   descriptorSetMap;
};

VERA_NAMESPACE_END
