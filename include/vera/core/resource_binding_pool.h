#pragma once

#include "core_object.h"

VERA_NAMESPACE_BEGIN

class Device;
class ResourceLayout;
class ResourceBinding;

class ResourceBindingPool : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ResourceBindingPool)
public:
	static obj<ResourceBindingPool> create(obj<Device> device);
	~ResourceBindingPool();

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;

	// allocate a new empty ResourceBinding
	VERA_NODISCARD obj<ResourceBinding> allocateBinding(
		const_ref<ResourceLayout> layout);

	VERA_NODISCARD obj<ResourceBinding> allocateBinding(
		const_ref<ResourceLayout> layout,
		uint32_t                  variable_binding_count = 1);

	// request a cached ResourceBinding, if not exists, create a new one
	// ResourceBinding from allocateBinding is not cached
	VERA_NODISCARD obj<ResourceBinding> requestBinding(
		const_ref<ResourceLayout>       layout,
		array_view<ResourceBindingInfo> binding_infos);

	void reset();
};

VERA_NAMESPACE_END