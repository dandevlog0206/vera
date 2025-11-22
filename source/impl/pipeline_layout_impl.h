#pragma once

#include "shader_impl.h"
#include "../../include/vera/core/pipeline_layout.h"

VERA_NAMESPACE_BEGIN

class PipelineLayoutImpl
{
public:
	obj<Device>                           device                 = {};

	vk::PipelineLayout                    vkPipelineLayout       = {};

	std::vector<obj<DescriptorSetLayout>> descriptorSetLayouts   = {};
	std::vector<PushConstantRange>        pushConstantRanges     = {};
	hash_t                                hashValue              = {};
	hash_t                                hashValueByReflections = {};
	hash_t                                hashValueByShaders     = {};
};

VERA_NAMESPACE_END
