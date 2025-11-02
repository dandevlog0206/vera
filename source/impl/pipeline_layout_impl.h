#pragma once

#include "shader_impl.h"
#include "detail/shader_reflection.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../../include/vera/util/string_pool.h"

VERA_NAMESPACE_BEGIN

struct PipelineLayoutImpl
{
	obj<Device>                           device               = {};

	vk::PipelineLayout                    pipelineLayout       = {};

	ShaderReflection                      reflection           = {};
	std::vector<obj<DescriptorSetLayout>> descriptorSetLayouts = {};
	std::vector<PushConstantRange>        pushConstantRanges   = {};
	PipelineBindPoint                     pipelineBindPoint    = {};
	ShaderStageFlags                      stageFlags           = {};
	string_pool                           namePool             = {};
	hash_t                                hashValue            = {};
	hash_t                                shaderHashValue      = {};
};

static vk::PushConstantRange get_vk_push_constant_range(const PushConstantRange& range)
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
