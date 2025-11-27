#pragma once

#include "device.h"

VERA_NAMESPACE_BEGIN

struct DescriptorSetLayoutBinding
{
	DescriptorSetLayoutBindingFlags flags;
	uint32_t                        binding;
	DescriptorType                  descriptorType;
	uint32_t                        descriptorCount;
	ShaderStageFlags                stageFlags;
};

struct DescriptorSetLayoutCreateInfo
{
	DescriptorSetLayoutCreateFlags          flags    = {};
	std::vector<DescriptorSetLayoutBinding> bindings = {};
};

class DescriptorSetLayout : public CoreObject
{
	VERA_CORE_OBJECT_INIT(DescriptorSetLayout)
public:
	static obj<DescriptorSetLayout> create(obj<Device> device, const DescriptorSetLayoutCreateInfo& info);
	~DescriptorSetLayout() VERA_NOEXCEPT override;

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;

	VERA_NODISCARD uint32_t getBindingCount() const VERA_NOEXCEPT;
	VERA_NODISCARD const DescriptorSetLayoutBinding& getBinding(uint32_t binding) const VERA_NOEXCEPT;
	VERA_NODISCARD const std::vector<DescriptorSetLayoutBinding>& getBindings() const VERA_NOEXCEPT;

	VERA_NODISCARD uint64_t hash() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END