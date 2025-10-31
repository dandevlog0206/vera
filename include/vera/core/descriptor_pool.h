#pragma once

#include "core_object.h"
#include "../util/array_view.h"

VERA_NAMESPACE_BEGIN

class Device;
class DescriptorSetLayout;
class DescriptorSet;
class DescriptorBindingInfo;

class DescriptorPool : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(DescriptorPool)
public:
	static obj<DescriptorPool> create(obj<Device> device);
	~DescriptorPool();

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;

	// allocate a new empty DescriptorSet
	VERA_NODISCARD obj<DescriptorSet> allocate(const_ref<DescriptorSetLayout> layout);

	VERA_NODISCARD obj<DescriptorSet> allocate(
		const_ref<DescriptorSetLayout> layout,
		uint32_t                       variable_descriptor_count);

	// request a cached DescriptorSet, if not exists, create a new one
	// DescriptorSet from allocate is not cached
	VERA_NODISCARD obj<DescriptorSet> requestDescriptorSet(
		const_ref<DescriptorSetLayout>    layout,
		array_view<DescriptorBindingInfo> binding_infos);

	void reset();
};

VERA_NAMESPACE_END