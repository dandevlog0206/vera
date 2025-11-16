#pragma once

#include "shader_impl.h"
#include "../spirv/reflection_desc.h"
#include "../../include/vera/core/pipeline_layout.h"

VERA_NAMESPACE_BEGIN

class PipelineLayoutImpl
{
public:
	obj<Device>                           device               = {};
	obj<ShaderReflection>                 shaderReflection     = {};

	vk::PipelineLayout                    pipelineLayout       = {};

	std::vector<obj<DescriptorSetLayout>> descriptorSetLayouts = {};
	std::vector<PushConstantRange>        pushConstantRanges   = {};
	hash_t                                hashValue            = {};
	hash_t                                hashValueWithShader  = {};
};

VERA_NAMESPACE_END
