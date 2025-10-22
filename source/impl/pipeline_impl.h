#pragma once

#include "object_impl.h"

#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/pipeline.h"

VERA_NAMESPACE_BEGIN

enum class PipelineBindPoint VERA_ENUM;

struct PipelineImpl
{
	using shader_pair = std::pair<ShaderStageFlagBits, obj<Shader>>;

	obj<Device>              device;
	obj<PipelineLayout>      pipelineLayout;

	vk::Pipeline             pipeline;

	std::vector<shader_pair> shaders;
	PipelineBindPoint        pipelineBindPoint;
	hash_t                   hashValue;
};

static vk::PrimitiveTopology to_vk_primitive_topology(PrimitiveTopology topology)
{
	switch (topology) {
	case PrimitiveTopology::PointList:                  return vk::PrimitiveTopology::ePointList;
	case PrimitiveTopology::LineList:                   return vk::PrimitiveTopology::eLineList;
	case PrimitiveTopology::LineStrip:                  return vk::PrimitiveTopology::eLineStrip;
	case PrimitiveTopology::TriangleList:               return vk::PrimitiveTopology::eTriangleList;
	case PrimitiveTopology::TriangleStrip:              return vk::PrimitiveTopology::eTriangleStrip;
	case PrimitiveTopology::TriangleFan:                return vk::PrimitiveTopology::eTriangleFan;
	case PrimitiveTopology::LineListWithAdjacency:      return vk::PrimitiveTopology::eLineListWithAdjacency;
	case PrimitiveTopology::LineStripWithAdjacency:     return vk::PrimitiveTopology::eLineStripWithAdjacency;
	case PrimitiveTopology::TriangleListWithAdjacency:  return vk::PrimitiveTopology::eTriangleListWithAdjacency;
	case PrimitiveTopology::TriangleStripWithAdjacency: return vk::PrimitiveTopology::eTriangleStripWithAdjacency;
	case PrimitiveTopology::PatchList:                  return vk::PrimitiveTopology::ePatchList;
	}

	VERA_ASSERT_MSG(false, "invalid primitive topology");
	return {};
}

static vk::PolygonMode to_vk_polygon_mode(PolygonMode mode)
{
	switch (mode) {
	case PolygonMode::Fill:            return vk::PolygonMode::eFill;
	case PolygonMode::Line:            return vk::PolygonMode::eLine;
	case PolygonMode::Point:           return vk::PolygonMode::ePoint;
	}

	VERA_ASSERT_MSG(false, "invalid polygon mode");
	return {};
}

static vk::CullModeFlags to_vk_cull_mode(CullModeFlags flags)
{
	return vk::CullModeFlags(static_cast<vk::CullModeFlags>(flags));
}

static vk::FrontFace to_vk_front_face(FrontFace mode)
{
	switch (mode) {
	case FrontFace::CounterClockwise: return vk::FrontFace::eCounterClockwise;
	case FrontFace::Clockwise:        return vk::FrontFace::eClockwise;
	}

	VERA_ASSERT_MSG(false, "invalid front face");
	return {};
}

static vk::CompareOp to_vk_compare_op(CompareOp op)
{
	switch (op) {
	case CompareOp::Never:          return vk::CompareOp::eNever;
	case CompareOp::Less:           return vk::CompareOp::eLess;
	case CompareOp::Equal:          return vk::CompareOp::eEqual;
	case CompareOp::LessOrEqual:    return vk::CompareOp::eLessOrEqual;
	case CompareOp::Greater:        return vk::CompareOp::eGreater;
	case CompareOp::NotEqual:       return vk::CompareOp::eNotEqual;
	case CompareOp::GreaterOrEqual: return vk::CompareOp::eGreaterOrEqual;
	case CompareOp::Always:         return vk::CompareOp::eAlways;
	}

	VERA_ASSERT_MSG(false, "invalid compare op");
	return {};
}

static vk::StencilOp to_vk_stencil_op(StencilOp op)
{
	switch (op) {
	case StencilOp::Keep:             return vk::StencilOp::eKeep;
	case StencilOp::Zero:             return vk::StencilOp::eZero;
	case StencilOp::Replace:          return vk::StencilOp::eReplace;
	case StencilOp::IncrementAndClamp:return vk::StencilOp::eIncrementAndClamp;
	case StencilOp::DecrementAndClamp:return vk::StencilOp::eDecrementAndClamp;
	case StencilOp::Invert:           return vk::StencilOp::eInvert;
	case StencilOp::IncrementAndWrap: return vk::StencilOp::eIncrementAndWrap;
	case StencilOp::DecrementAndWrap: return vk::StencilOp::eDecrementAndWrap;
	}

	VERA_ASSERT_MSG(false, "invalid stencil op");
	return {};
}

static vk::LogicOp to_vk_logic_op(LogicOp op)
{
	switch (op) {
	case LogicOp::Clear:        return vk::LogicOp::eClear;
	case LogicOp::And:          return vk::LogicOp::eAnd;
	case LogicOp::AndReverse:   return vk::LogicOp::eAndReverse;
	case LogicOp::Copy:         return vk::LogicOp::eCopy;
	case LogicOp::AndInverted:  return vk::LogicOp::eAndInverted;
	case LogicOp::NoOp:         return vk::LogicOp::eNoOp;
	case LogicOp::Xor:          return vk::LogicOp::eXor;
	case LogicOp::Or:           return vk::LogicOp::eOr;
	case LogicOp::Nor:          return vk::LogicOp::eNor;
	case LogicOp::Equivalent:   return vk::LogicOp::eEquivalent;
	case LogicOp::Invert:       return vk::LogicOp::eInvert;
	case LogicOp::OrReverse:    return vk::LogicOp::eOrReverse;
	case LogicOp::CopyInverted: return vk::LogicOp::eCopyInverted;
	case LogicOp::OrInverted:   return vk::LogicOp::eOrInverted;
	case LogicOp::Nand:         return vk::LogicOp::eNand;
	case LogicOp::Set:          return vk::LogicOp::eSet;
	}

	VERA_ASSERT_MSG(false, "invalid logic op");
	return {};
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

static vk::PipelineStageFlags to_vk_pipeline_stage_flags(PipelineStageFlags flags)
{
	vk::PipelineStageFlags result;

	if (flags.has(PipelineStageFlagBits::TopOfPipe))
		result |= vk::PipelineStageFlagBits::eTopOfPipe;
	if (flags.has(PipelineStageFlagBits::DrawIndirect))
		result |= vk::PipelineStageFlagBits::eDrawIndirect;
	if (flags.has(PipelineStageFlagBits::VertexInput))
		result |= vk::PipelineStageFlagBits::eVertexInput;
	if (flags.has(PipelineStageFlagBits::VertexShader))
		result |= vk::PipelineStageFlagBits::eVertexShader;
	if (flags.has(PipelineStageFlagBits::TessellationControlShader))
		result |= vk::PipelineStageFlagBits::eTessellationControlShader;
	if (flags.has(PipelineStageFlagBits::TessellationEvaluationShader))
		result |= vk::PipelineStageFlagBits::eTessellationEvaluationShader;
	if (flags.has(PipelineStageFlagBits::GeometryShader))
		result |= vk::PipelineStageFlagBits::eGeometryShader;
	if (flags.has(PipelineStageFlagBits::FragmentShader))
		result |= vk::PipelineStageFlagBits::eFragmentShader;
	if (flags.has(PipelineStageFlagBits::EarlyFragmentTests))
		result |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
	if (flags.has(PipelineStageFlagBits::LateFragmentTests))
		result |= vk::PipelineStageFlagBits::eLateFragmentTests;
	if (flags.has(PipelineStageFlagBits::ColorAttachmentOutput))
		result |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
	if (flags.has(PipelineStageFlagBits::ComputeShader))
		result |= vk::PipelineStageFlagBits::eComputeShader;
	if (flags.has(PipelineStageFlagBits::Transfer))
		result |= vk::PipelineStageFlagBits::eTransfer;
	if (flags.has(PipelineStageFlagBits::BottomOfPipe))
		result |= vk::PipelineStageFlagBits::eBottomOfPipe;
	if (flags.has(PipelineStageFlagBits::Host))
		result |= vk::PipelineStageFlagBits::eHost;
	if (flags.has(PipelineStageFlagBits::AllGraphics))
		result |= vk::PipelineStageFlagBits::eAllGraphics;

	return result;
}

VERA_NAMESPACE_END