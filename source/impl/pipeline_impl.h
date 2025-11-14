#pragma once

#include "object_impl.h"

#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/pipeline.h"

VERA_NAMESPACE_BEGIN

class PipelineImpl
{
public:
	using shader_pair = std::pair<ShaderStageFlagBits, obj<Shader>>;

	obj<Device>              device            = {};
	obj<PipelineLayout>      pipelineLayout    = {};

	vk::Pipeline             pipeline          = {};

	std::vector<shader_pair> shaders           = {};
	PipelineBindPoint        pipelineBindPoint = {};
	hash_t                   hashValue         = {};
};

static vk::StencilOpState to_vk_stencil_op_state(const StencilOpState& state)
{
	vk::StencilOpState result;
	result.failOp      = to_vk_stencil_op(state.failOp);
	result.passOp      = to_vk_stencil_op(state.passOp);
	result.depthFailOp = to_vk_stencil_op(state.depthFailOp);
	result.compareOp   = to_vk_compare_op(state.compareOp);
	result.compareMask = state.compareMask;
	result.writeMask   = state.writeMask;
	result.reference   = state.reference;

	return result;
}

static vk::PushConstantRange get_vk_push_constant_range(const PushConstantRange& range)
{
	vk::PushConstantRange result;
	result.offset     = range.offset;
	result.size       = range.size;
	result.stageFlags = to_vk_shader_stage_flags(range.stageFlags);

	return result;
}

VERA_NAMESPACE_END