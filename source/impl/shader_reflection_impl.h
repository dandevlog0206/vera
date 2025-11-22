#pragma once

#include "object_impl.h"
#include "../spirv/reflection_node.h"
#include "../../include/vera/core/shader_reflection.h"
#include <memory_resource>

VERA_NAMESPACE_BEGIN

class ShaderReflectionImpl
{
public:
	obj<Device>                                    device             = {};

	std::pmr::monotonic_buffer_resource            memory             = {};
	ShaderStageFlags                               stageFlags         = {};
	Version                                        spirvVersion       = {};
	std::string_view                               entryPointName     = {};
	uint3                                          localSize          = {};
	array_view<const ReflectionInterfaceVariable*> interfaceVariables = {};
	array_view<const ReflectionInterfaceVariable*> inputVariables     = {};
	array_view<const ReflectionInterfaceVariable*> outputVariables    = {};
	array_view<const ReflectionDescriptorBinding*> descriptorBindings = {};
	array_view<const ReflectionDescriptorSet*>     descriptorSets     = {};
	const ReflectionBlockVariable*                 pushConstantBlock  = {};
	array_view<ReflectionSpecConstant>             specConstants      = {};
	const ReflectionRootNode*                      rootNode           = {};
	hash_t                                         hashValue          = {};
};

VERA_NAMESPACE_END