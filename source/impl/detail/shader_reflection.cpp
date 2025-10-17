#include "shader_reflection.h"

VERA_NAMESPACE_BEGIN

static void destroy_impl(ReflectionDesc* ptr)
{
	if (!ptr) return;

	switch (ptr->type) {
	case ReflectionType::Primitive: {
		auto* desc = static_cast<ReflectionPrimitiveDesc*>(ptr);
		delete desc;
	} break;
	case ReflectionType::Array: {
		auto* desc = static_cast<ReflectionArrayDesc*>(ptr);
		destroy_impl(desc->element);
		delete desc;
	} break;
	case ReflectionType::Struct: {
		auto* desc = static_cast<ReflectionStructDesc*>(ptr);
		for (uint32_t i = 0; i < desc->memberCount; ++i)
			destroy_impl(desc->members[i]);
		delete[] desc->members;
		delete[] desc->nameMaps;
		delete desc;
	} break;
	case ReflectionType::Resource: {
		auto* desc = static_cast<ReflectionResourceDesc*>(ptr);
		delete desc;
	} break;
	case ReflectionType::ResourceBlock: {
		auto* desc = static_cast<ReflectionResourceBlockDesc*>(ptr);
		for (uint32_t i = 0; i < desc->memberCount; ++i)
			destroy_impl(desc->members[i]);
		delete[] desc->members;
		delete[] desc->nameMaps;
		delete desc;
	} break;
	case ReflectionType::ResourceArray: {
		auto* desc = static_cast<ReflectionResourceArrayDesc*>(ptr);
		destroy_impl(desc->element);
		delete desc;
	} break;
	case ReflectionType::PushConstant: {
		auto* desc = static_cast<ReflectionPushConstantDesc*>(ptr);
		for (uint32_t i = 0; i < desc->memberCount; ++i)
			destroy_impl(desc->members[i]);
		delete[] desc->members;
		delete[] desc->nameMaps;
		delete desc;
	} break;
	}
}

void destroy_shader_reflection(ShaderReflection& reflection)
{
	for (uint32_t i = 0; i < reflection.reflectionCount; ++i)
		destroy_impl(reflection.reflections[i]);
	delete[] reflection.reflections;
	delete[] reflection.nameMaps;
	delete[] reflection.setRanges;
}

VERA_NAMESPACE_END