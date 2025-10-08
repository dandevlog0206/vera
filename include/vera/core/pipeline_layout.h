#pragma once

#include "shader.h"
#include "../util/array_view.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class Device;
class ResourceLayout;

enum class PipelineBindPoint VERA_ENUM
{
	Graphics,
	Compute
};

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
	PipelineBindPoint                pipelineBindPoint;
};

class PipelineLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(PipelineLayout)
public:
	static obj<PipelineLayout> create(obj<Device> device, const PipelineLayoutCreateInfo& info);
	~PipelineLayout();

	obj<Device> getDevice();

	uint32_t getResourceLayoutCount() const;
	const_ref<ResourceLayout> getResourceLayout(uint32_t idx) const;
	array_view<ref<ResourceLayout>> getResourceLayouts() const;

	array_view<PushConstantRange> getPushConstantRanges() const;

	size_t hash() const;
};

VERA_NAMESPACE_END