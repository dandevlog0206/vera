#pragma once

#include "object_impl.h"
#include "../spirv/reflection_desc.h"

VERA_NAMESPACE_BEGIN

class ShaderReflectionImpl
{
public:
	mutable ReflectionDesc                     reflectionDesc;
	mutable std::vector<uint32_t>              spirvCode;
	mutable std::vector<obj<ShaderReflection>> shaderReflections;
	ReflectionTargetFlags                      targetFlags;

	const ReflectionRootNode* requestMinimalReflectionRootNode() const;
	const ReflectionRootNode* requestFullReflectionRootNode() const;

private:
	void parse(ReflectionParseMode mode);
	void merge(ReflectionParseMode mode);
};

VERA_NAMESPACE_END