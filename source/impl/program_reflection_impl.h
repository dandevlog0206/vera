#pragma once

#include "object_impl.h"
#include "../spirv/reflection_node.h"
#include <memory_resource>

VERA_NAMESPACE_BEGIN

class ProgramReflectionImpl
{
public:
	obj<Device>                         device            = {};
	mutable obj<PipelineLayout>         pipelineLayout    = {};
	std::vector<obj<ShaderReflection>>  shaderReflections = {};

	std::pmr::monotonic_buffer_resource memory            = {};
	ShaderStageFlags                    shaderStageFlags  = {};
	PipelineBindPoint                   pipelineBindPoint = {};
	array_view<ReflectionEntryPoint>    entryPoints       = {};
	const ReflectionRootNode*           rootNode          = {};
	hash_t                              hashValue         = {};
};

VERA_NAMESPACE_END
