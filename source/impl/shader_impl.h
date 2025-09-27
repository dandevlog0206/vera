#pragma once

#include "shader_reflection_impl.h"

#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/pipeline_layout.h"
#include <unordered_set>

VERA_NAMESPACE_BEGIN

struct ShaderImpl
{
	obj<Device>                      device;

	vk::ShaderModule                 shader;

	std::vector<ReflectionDesc*>     reflections;
	std::unordered_set<std::string>  namePool;

	std::vector<obj<ResourceLayout>> resourceLayouts;
	std::vector<PushConstantRange>   pushConstantRanges;
	std::string                      entryPointName;
	ShaderStageFlags                 shaderStageFlags;
	size_t                           hashValue;
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
