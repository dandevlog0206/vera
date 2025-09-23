#include "../../include/vera/core/shader_reflection.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"

VERA_NAMESPACE_BEGIN

static bool has_same_device(std::span<ref<Shader>> shaders)
{
	auto& device = CoreObject::getImpl(shaders[0]).device;

	for (auto& shader : shaders) {
		if (CoreObject::getImpl(shader).device != device)
			return false;
	}
	return true;
}

static bool check_shader_stages(std::span<ref<Shader>> shaders)
{
	ShaderStageFlags all_flags;

	for (auto& shader : shaders) {
		auto flags = shader->getShaderStageFlags();

		if (all_flags.has(flags))
			return false;

		all_flags |= flags;
	}

	return true;
}

ref<ShaderReflection> ShaderReflection::create(std::vector<ref<Shader>> shaders)
{
	if (!has_same_device(shaders))
		throw Exception("can't reflect shaders from another devices");
	if (!check_shader_stages(shaders))
		throw Exception("can't reflect shaders with duplicate shader stages");

	// TODO: check duplicate reflection name

	auto  obj  = createNewObject<ShaderReflection>();
	auto& impl = getImpl(obj);
	
	impl.device = shaders[0]->getDevice();

	impl.shaders.reserve(shaders.size());
	for (auto& shader : shaders) {
		auto& shader_impl = getImpl(shader);

		impl.shaderStageFlags |= shader_impl.shaderStageFlags;
		impl.descriptors.insert(impl.descriptors.cend(), VERA_SPAN(shader_impl.reflections));
		impl.shaders.push_back(std::move(shader));
	}

	for (size_t i = 0; i < impl.descriptors.size(); ++i)
		impl.hashMap[impl.descriptors[i]->name] = static_cast<uint32_t>(i);

	return obj;
}

ShaderReflection::~ShaderReflection()
{
	destroyObjectImpl(this);
}

bool ShaderReflection::isCompatible(ref<Pipeline>& pipeline) const
{
	// TODO: implement

	return false;
}

VERA_NAMESPACE_END