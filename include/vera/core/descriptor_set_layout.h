#pragma once

#include "shader.h"
#include "../util/index_map.h"
#include <vector>

VERA_NAMESPACE_BEGIN

enum class DescriptorSetLayoutCreateFlagBits VERA_FLAG_BITS
{
	UpdateAfterBindPool = 1 << 0,
	PushBinding         = 1 << 1
} VERA_ENUM_FLAGS(DescriptorSetLayoutCreateFlagBits, DescriptorSetLayoutCreateFlags)

enum class DescriptorSetLayoutBindingFlagBits VERA_FLAG_BITS
{
	UpdateAfterBind          = 1 << 0,
	UpdateUnusedWhilePending = 1 << 1,
	PartiallyBound           = 1 << 2,
	VariableDescriptorCount  = 1 << 3
} VERA_ENUM_FLAGS(DescriptorSetLayoutBindingFlagBits, DescriptorSetLayoutBindingFlags)

enum class DescriptorType VERA_ENUM
{
	Unknown,
	Sampler,
	CombinedImageSampler,
	SampledImage,
	StorageImage,
	UniformTexelBuffer,
	StorageTexelBuffer,
	UniformBuffer,
	StorageBuffer,
	UniformBufferDynamic,
	StorageBufferDynamic,
	InputAttachment
};

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

class DescriptorSetLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(DescriptorSetLayout)
public:
	static obj<DescriptorSetLayout> create(obj<Device> device, const DescriptorSetLayoutCreateInfo& info);
	~DescriptorSetLayout();

	obj<Device> getDevice();

	const std::vector<DescriptorSetLayoutBinding>& getBindings() const;

	uint64_t hash() const;
};

VERA_NAMESPACE_END