#include "../../include/vera/core/program_reflection.h"
#include "../impl/device_impl.h"
#include "../impl/program_reflection_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/shader_reflection.h"

VERA_NAMESPACE_BEGIN

static hash_t hash_shader_reflections(
	array_view<obj<ShaderReflection>> shader_reflections
) {
	hash_t seed = 0;

	for (const auto& shader_reflection : shader_reflections)
		hash_combine(seed, shader_reflection->hash());

	return seed;
}

obj<ProgramReflection> ProgramReflection::create(
	obj<Device>                       device,
	array_view<obj<ShaderReflection>> shader_reflections
) {
	auto& device_impl = getImpl(device);
	auto  hash_value  = hash_shader_reflections(shader_reflections);

	if (auto cached_obj = device_impl.findCachedObject<ProgramReflection>(hash_value))
		return cached_obj;

	auto  obj  = createNewCoreObject<ProgramReflection>();
	auto& impl = getImpl(obj);

	impl.device = std::move(device);
	impl.shaderReflections.assign(VERA_SPAN(shader_reflections));

	device_impl.registerCachedObject<ProgramReflection>(hash_value, obj);

	return obj;
}

ProgramReflection::~ProgramReflection() VERA_NOEXCEPT
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterCachedObject<ProgramReflection>(impl.hashValue);

	destroyObjectImpl(this);
}

VERA_NAMESPACE_END
