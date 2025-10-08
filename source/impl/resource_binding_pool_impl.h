#pragma once

#include "object_impl.h"
#include <unordered_map>

VERA_NAMESPACE_BEGIN

struct ResourceBindingPoolImpl
{
	using ResourceBindingMap = std::unordered_map<uint64_t, ref<ResourceBinding>>;

	obj<Device>        device;

	vk::DescriptorPool descriptorPool;

	ResourceBindingMap bindingMap;
};

VERA_NAMESPACE_END
