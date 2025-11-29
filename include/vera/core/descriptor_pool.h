#pragma once

#include "descriptor_set_layout.h"
#include "descriptor_set.h"
#include "../util/array_view.h"

VERA_NAMESPACE_BEGIN

VERA_VK_ABI_COMPATIBLE struct DescriptorPoolSize
{
	DescriptorType type;
	uint32_t       size;
};

// if poolSizes is empty, the pool will be created with default sizes
// also if maxSets is zero, which means default maxSets value will be used
struct DescriptorPoolCreateInfo
{
	DescriptorPoolCreateFlags      flags     = {};
	array_view<DescriptorPoolSize> poolSizes = {};
	uint32_t                       maxSets   = 0;
};

class DescriptorPool : public CoreObject
{
	VERA_CORE_OBJECT_INIT(DescriptorPool)
public:
	static obj<DescriptorPool> create(obj<Device> device, const DescriptorPoolCreateInfo& info = {});
	~DescriptorPool() VERA_NOEXCEPT override;

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;

	VERA_NODISCARD DescriptorPoolCreateFlags getFlags() const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<DescriptorPoolSize> enumeratePoolSizes() const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t getMaxSets() const VERA_NOEXCEPT;

	VERA_NODISCARD obj<DescriptorSet> allocate(obj<DescriptorSetLayout> layout);
	VERA_NODISCARD obj<DescriptorSet> allocate(obj<DescriptorSetLayout> layout, uint32_t variable_descriptor_count);

	void reset();
};

VERA_NAMESPACE_END