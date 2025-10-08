#pragma once

#include "shader_impl.h"

#include "../../include/vera/core/pipeline_layout.h"

VERA_NAMESPACE_BEGIN

struct PipelineLayoutImpl
{
	obj<Device>                      device;

	vk::PipelineLayout               pipelineLayout;

	size_t                           hashValue;
	std::vector<obj<ResourceLayout>> resourceLayout;
	std::vector<PushConstantRange>   pushConstantRanges;
	PipelineBindPoint                pipelineBindPoint;
};

static vk::PushConstantRange get_push_constant_range(const PushConstantRange& range)
{
	vk::PushConstantRange result;
	result.offset     = range.offset;
	result.size       = range.size;
	result.stageFlags = to_vk_shader_stage_flags(range.stageFlags);

	return result;
}

static vk::PipelineBindPoint to_vk_pipeline_bind_point(PipelineBindPoint bind_point)
{
	switch (bind_point) {
	case PipelineBindPoint::Graphics: return vk::PipelineBindPoint::eGraphics;
	case PipelineBindPoint::Compute:  return vk::PipelineBindPoint::eCompute;
	}

	VERA_ASSERT_MSG(false, "invalid pipeline bind point");
	return {};
}

VERA_NAMESPACE_END
