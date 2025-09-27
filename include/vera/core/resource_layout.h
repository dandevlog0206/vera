#pragma once

#include "shader.h"
#include <vector>

VERA_NAMESPACE_BEGIN

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
	uint32_t         binding;
	ResourceType     resourceType;
	uint32_t         resourceCount;
	ShaderStageFlags stageFlags;
};

class ResourceLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ResourceLayout)
public:
	static obj<ResourceLayout> create(obj<Device> device, const std::vector<ResourceLayoutBinding>& bindings);
	~ResourceLayout();

	obj<Device> getDevice();

	const std::vector<ResourceLayoutBinding>& getBindings() const;

	uint64_t hash() const;
};

VERA_NAMESPACE_END