#pragma once

#include "shader_impl.h"

#include "../../include/vera/core/pipeline_layout.h"

VERA_NAMESPACE_BEGIN

struct PipelineLayoutImpl
{
	ref<Device>                      device;

	vk::PipelineLayout               layout;

	size_t                           hashValue;
	std::vector<ref<ResourceLayout>> resourceLayout;
	std::vector<PushConstantRange>   pushConstantRanges;
};

static vk::PushConstantRange get_push_constant_range(const PushConstantRange& range)
{
	vk::PushConstantRange result;
	result.offset     = range.offset;
	result.size       = range.size;
	result.stageFlags = to_vk_shader_stage_flags(range.stageFlags);

	return result;
}

VERA_NAMESPACE_END
