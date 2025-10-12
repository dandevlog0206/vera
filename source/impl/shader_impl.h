#pragma once

#include "object_impl.h"
#include "detail/shader_reflection.h"
#include "../../include/vera/core/shader.h"
#include "../../../include/vera/util/string_pool.h"

VERA_NAMESPACE_BEGIN

enum class PipelineBindPoint VERA_ENUM;

struct PushConstantRange;

struct ShaderImpl
{
	using PushConstantRanges = std::vector<PushConstantRange>;

	obj<Device>        device;

	vk::ShaderModule   shader;

	ShaderReflection   reflection;
	PushConstantRanges pushConstantRanges;
	PipelineBindPoint  pipelineBindPoint;
	ShaderStageFlags   stageFlags;
	std::string_view   entryPointName;
	string_pool        namePool;
	size_t             hashValue;
};

static vk::ShaderStageFlagBits to_vk_shader_stage(ShaderStageFlagBits flags)
{
	switch (flags) {
	case ShaderStageFlagBits::Vertex:                 return vk::ShaderStageFlagBits::eVertex;
	case ShaderStageFlagBits::TessellationControl:    return vk::ShaderStageFlagBits::eTessellationControl;
	case ShaderStageFlagBits::TessellationEvaluation: return vk::ShaderStageFlagBits::eTessellationEvaluation;
	case ShaderStageFlagBits::Geometry:               return vk::ShaderStageFlagBits::eGeometry;
	case ShaderStageFlagBits::Fragment:               return vk::ShaderStageFlagBits::eFragment;
	case ShaderStageFlagBits::Compute:                return vk::ShaderStageFlagBits::eCompute;
	}

	VERA_ASSERT_MSG(false, "invalid shader stage");
	return {};
}

static vk::ShaderStageFlags to_vk_shader_stage_flags(ShaderStageFlags flags)
{
	vk::ShaderStageFlags result;

	if (flags.has(ShaderStageFlagBits::Vertex))
		result |= vk::ShaderStageFlagBits::eVertex;
	if (flags.has(ShaderStageFlagBits::TessellationControl))
		result |= vk::ShaderStageFlagBits::eTessellationControl;
	if (flags.has(ShaderStageFlagBits::TessellationEvaluation))
		result |= vk::ShaderStageFlagBits::eTessellationEvaluation;
	if (flags.has(ShaderStageFlagBits::Geometry))
		result |= vk::ShaderStageFlagBits::eGeometry;
	if (flags.has(ShaderStageFlagBits::Fragment))
		result |= vk::ShaderStageFlagBits::eFragment;
	if (flags.has(ShaderStageFlagBits::Compute))
		result |= vk::ShaderStageFlagBits::eCompute;

	return result;
}

VERA_NAMESPACE_END
