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

static vk::ShaderStageFlags to_vk_shader_stage_flags(ShaderStageFlags flags)
{
	// vr::ShaderStageFlags is VERA_VK_ABI_COMPATIBLE with vk::ShaderStageFlags
	return std::bit_cast<vk::ShaderStageFlags>(flags);
}

VERA_NAMESPACE_END
