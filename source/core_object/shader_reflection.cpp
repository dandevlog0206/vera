#include "../../include/vera/core/shader_reflection.h"
#include "../impl/shader_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/shader.h"
#include "../../include/vera/util/static_vector.h"

#define MAX_SHADER_STAGE_COUNT 16

VERA_NAMESPACE_BEGIN

const ReflectionDesc& get_reflection_desc(const_ref<ShaderReflection> shader_reflection) VERA_NOEXCEPT
{
	return CoreObject::getImpl(shader_reflection).reflection;
}

ReflectionDesc& get_reflection_desc(ref<ShaderReflection> shader_reflection) VERA_NOEXCEPT
{
	return CoreObject::getImpl(shader_reflection).reflection;
}

const ReflectionRootNode* get_reflection_root_node(const_ref<ShaderReflection> shader_reflection) VERA_NOEXCEPT
{
	return get_reflection_desc(shader_reflection).getRootNode();
}

obj<ShaderReflection> ShaderReflection::create(const ShaderReflectionBuilder& builder)
{
	return obj<ShaderReflection>();
}

obj<ShaderReflection> ShaderReflection::create(const_ref<PipelineLayout> pipeline_layout)
{
	return obj<ShaderReflection>();
}

obj<ShaderReflection> ShaderReflection::create(array_view<const_ref<Shader>> shaders)
{
	if (shaders.size() == 1) {
		auto& shader_impl = const_cast<ShaderImpl&>(getImpl(shaders[0]));
		return shader_impl.getOrCreateShaderReflection();
	}

	auto  obj  = createNewCoreObject<ShaderReflection>();
	auto& impl = getImpl(obj);

	static_vector<const ReflectionDesc*, MAX_SHADER_STAGE_COUNT> reflections;

	for (auto shader : shaders) {
		auto& shader_impl = const_cast<ShaderImpl&>(getImpl(shader));
		auto  reflection  = shader_impl.getOrCreateShaderReflection();
		reflections.push_back(&get_reflection_desc(reflection));
	}

	impl.reflection.merge(reflections);

	return obj;
}

obj<ShaderReflection> ShaderReflection::create(const_ref<Shader> shader)
{
	return const_cast<ShaderImpl&>(getImpl(shader)).getOrCreateShaderReflection();
}

obj<ShaderReflection> ShaderReflection::create(const uint32_t* spirv, size_t size)
{
	auto  obj  = createNewCoreObject<ShaderReflection>();
	auto& impl = getImpl(obj);

	impl.reflection.parse(spirv, size);

	return obj;
}

ShaderReflection::~ShaderReflection() VERA_NOEXCEPT
{
}

ShaderStageFlags ShaderReflection::getShaderStageFlags() const VERA_NOEXCEPT
{
	const auto* root_node = get_reflection_root_node(this);
	return root_node->getShaderStageFlags();
}

VERA_NAMESPACE_END