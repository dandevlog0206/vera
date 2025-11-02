#pragma once

#include "object_impl.h"

#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/pipeline.h"

VERA_NAMESPACE_BEGIN

enum class PipelineBindPoint VERA_ENUM;

struct PipelineImpl
{
	using shader_pair = std::pair<ShaderStageFlagBits, obj<Shader>>;

	obj<Device>              device            = {};
	obj<PipelineLayout>      pipelineLayout    = {};

	vk::Pipeline             pipeline          = {};

	std::vector<shader_pair> shaders           = {};
	PipelineBindPoint        pipelineBindPoint = {};
	hash_t                   hashValue         = {};
};

static vk::PrimitiveTopology to_vk_primitive_topology(PrimitiveTopology topology)
{
	// vr::PrimitiveTopology is VERA_VK_ABI_COMPATIBLE with vk::PrimitiveTopology
	return std::bit_cast<vk::PrimitiveTopology>(topology);
}

static vk::PolygonMode to_vk_polygon_mode(PolygonMode mode)
{
	// vr::PolygonMode is VERA_VK_ABI_COMPATIBLE with vk::PolygonMode
	return std::bit_cast<vk::PolygonMode>(mode);
}

static vk::CullModeFlags to_vk_cull_mode_flags(CullModeFlags flags)
{
	// vr::CullModeFlags is VERA_VK_ABI_COMPATIBLE with vk::CullModeFlags
	return std::bit_cast<vk::CullModeFlags>(flags);
}

static vk::FrontFace to_vk_front_face(FrontFace mode)
{
	// vr::FrontFace is VERA_VK_ABI_COMPATIBLE with vk::FrontFace
	return std::bit_cast<vk::FrontFace>(mode);
}

static vk::CompareOp to_vk_compare_op(CompareOp op)
{
	// vr::CompareOp is VERA_VK_ABI_COMPATIBLE with vk::CompareOp
	return std::bit_cast<vk::CompareOp>(op);
}

static vk::StencilOp to_vk_stencil_op(StencilOp op)
{
	// vr::StencilOp is VERA_VK_ABI_COMPATIBLE with vk::StencilOp
	return std::bit_cast<vk::StencilOp>(op);
}

static vk::LogicOp to_vk_logic_op(LogicOp op)
{
	// vr::LogicOp is VERA_VK_ABI_COMPATIBLE with vk::LogicOp
	return std::bit_cast<vk::LogicOp>(op);
}

static vk::BlendFactor to_vk_blend_factor(BlendFactor factor)
{
	// vr::BlendFactor is VERA_VK_ABI_COMPATIBLE with vk::BlendFactor
	return std::bit_cast<vk::BlendFactor>(factor);
}

static vk::BlendOp to_vk_blend_op(BlendOp op)
{
	// vr::BlendOp is VERA_VK_ABI_COMPATIBLE with vk::BlendOp
	return std::bit_cast<vk::BlendOp>(op);
}

static vk::ColorComponentFlags to_vk_color_component_flags(ColorComponentFlags flags)
{
	// vr::ColorComponentFlags is VERA_VK_ABI_COMPATIBLE with vk::ColorComponentFlags
	return std::bit_cast<vk::ColorComponentFlags>(flags);
}

static vk::PipelineStageFlags to_vk_pipeline_stage_flags(PipelineStageFlags flags)
{
	// vr::PipelineStageFlags is VERA_VK_ABI_COMPATIBLE with vk::PipelineStageFlags
	return std::bit_cast<vk::PipelineStageFlags>(flags);
}

static vk::DynamicState to_vk_dynamic_state(DynamicState state)
{
	// vr::DynamicState is VERA_VK_ABI_COMPATIBLE with vk::DynamicState
	return std::bit_cast<vk::DynamicState>(state);
}

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

VERA_NAMESPACE_END