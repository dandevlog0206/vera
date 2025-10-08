#pragma once

#include "shader.h"
#include "../util/index_map.h"
#include <vector>

VERA_NAMESPACE_BEGIN

enum class ResourceLayoutCreateFlagBits VERA_FLAG_BITS
{
	UpdateAfterBindPool = 1 << 0,
	PushBinding         = 1 << 1
} VERA_ENUM_FLAGS(ResourceLayoutCreateFlagBits, ResourceLayoutCreateFlags)

enum class ResourceLayoutBindingFlagBits VERA_FLAG_BITS
{
	UpdateAfterBind          = 1 << 0,
	UpdateUnusedWhilePending = 1 << 1,
	PartiallyBound           = 1 << 2,
	VariableBindingCount     = 1 << 3
} VERA_ENUM_FLAGS(ResourceLayoutBindingFlagBits, ResourceLayoutBindingFlags)

enum class ResourceType VERA_ENUM
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

struct ResourceLayoutBinding
{
	ResourceLayoutBindingFlags flags;
	uint32_t                   binding;
	ResourceType               resourceType;
	uint32_t                   resourceCount;
	ShaderStageFlags           stageFlags;
};

struct ResourceLayoutCreateInfo
{
	ResourceLayoutCreateFlags          flags    = {};
	std::vector<ResourceLayoutBinding> bindings = {};
};

class ResourceLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ResourceLayout)
public:
	static obj<ResourceLayout> create(obj<Device> device, const ResourceLayoutCreateInfo& info);
	~ResourceLayout();

	obj<Device> getDevice();

	const std::vector<ResourceLayoutBinding>& getBindings() const;

	uint64_t hash() const;
};

VERA_NAMESPACE_END