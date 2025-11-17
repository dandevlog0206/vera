#pragma once

#include "object_impl.h"
#include "../../include/vera/core/descriptor_set_layout.h"

VERA_NAMESPACE_BEGIN

class DescriptorSetLayoutImpl
{
public:
	using LayoutBindings = std::vector<DescriptorSetLayoutBinding>;
	using BindingMap     = std::unordered_map<uint32_t, DescriptorSetLayoutBinding*>;

	obj<Device>              device                = {};

	vk::DescriptorSetLayout  vkDescriptorSetLayout = {};

	hash_t                   hashValue             = {};
	LayoutBindings           bindings              = {};
	BindingMap               bindingMap            = {};
};

VERA_NAMESPACE_END