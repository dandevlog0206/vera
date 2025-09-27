#pragma once

#include "shader.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class Device;
class ResourceLayout;

struct PushConstantRange
{
	uint32_t         offset;
	uint32_t         size;
	ShaderStageFlags stageFlags;
};

struct PipelineLayoutCreateInfo
{
	std::vector<obj<ResourceLayout>> resourceLayouts;
	std::vector<PushConstantRange>   pushConstantRanges;
};

class PipelineLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(PipelineLayout)
public:
	static obj<PipelineLayout> create(obj<Device> device, const PipelineLayoutCreateInfo& info);
	~PipelineLayout();

	obj<Device> getDevice();

	const std::vector<obj<ResourceLayout>>& getResourceLayouts() const;
	const std::vector<PushConstantRange>& getPushConstantRanges() const;

	size_t hash() const;
};

VERA_NAMESPACE_END