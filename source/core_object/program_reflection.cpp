#include "../../include/vera/core/program_reflection.h"
#include "../impl/device_impl.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/program_reflection_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

enum
{
	MAX_SHADER_STAGE_COUNT = 16
};

typedef static_vector<const ReflectionRootNode*, MAX_SHADER_STAGE_COUNT> PerStageReflectionRootNodeArray;

template <class T>
static T* allocate(
	ProgramReflectionImpl& impl,
	size_t                 count = 1
) {
	auto* ptr = reinterpret_cast<T*>(impl.memory.allocate(sizeof(T) * count, alignof(T)));
	std::uninitialized_default_construct_n(ptr, count);
	return ptr;
}

static std::string_view copy_string(
	ProgramReflectionImpl& impl,
	const char*            str
) {
	if (!str)
		return std::string_view();

	if (size_t len = strlen(str); len > 0) {
		auto* ptr = reinterpret_cast<char*>(impl.memory.allocate(len + 1, alignof(char)));
		memcpy(ptr, str, len);
		ptr[len] = '\0';

		return std::string_view(ptr, len);
	}

	return std::string_view();
}

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

	PerStageReflectionRootNodeArray root_nodes;
	ShaderStageFlags                stage_flags;

	auto*    entry_points     = allocate<ReflectionEntryPoint>(impl, shader_reflections.size());
	uint32_t entry_point_idx  = 0;
	auto     reflection_crefs = array_view(
		reinterpret_cast<const const_ref<ShaderReflection>*>(shader_reflections.data()),
		shader_reflections.size());

	for (auto& shader_reflections : shader_reflections) {
		const auto& refl_impl = getImpl(shader_reflections);

		if (stage_flags.has(refl_impl.stageFlags))
			throw Exception("duplicate shader stage in program reflection");
		stage_flags |= refl_impl.stageFlags;

		root_nodes.push_back(refl_impl.rootNode);
		entry_points[entry_point_idx].stageFlags = refl_impl.stageFlags;
		entry_points[entry_point_idx].name       = copy_string(impl, refl_impl.entryPointName.data());
		entry_point_idx++;
	}
	
	impl.device           = std::move(device);
	impl.pipelineLayout   = PipelineLayout::create(impl.device, reflection_crefs);
	impl.shaderReflections.assign(VERA_SPAN(shader_reflections));
	impl.shaderStageFlags = stage_flags;
	impl.entryPoints      = array_view{ entry_points, entry_point_idx };
	impl.rootNode         = ReflectionRootNode::merge(root_nodes, &impl.memory);
	impl.hashValue        = hash_value;

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

obj<Device> ProgramReflection::getDevice() const VERA_NOEXCEPT
{
	return getImpl(this).device;
}

array_view<obj<ShaderReflection>> ProgramReflection::enumerateShaderReflections() const VERA_NOEXCEPT
{
	return getImpl(this).shaderReflections;
}

obj<ShaderReflection> ProgramReflection::getShaderReflection(ShaderStageFlags stage_flag) const VERA_NOEXCEPT
{
	if (stage_flag.count() != 1)
		throw Exception("getShaderReflection requires a single ShaderStageFlagBits");
	
	for (auto& reflection : getImpl(this).shaderReflections)
		if (reflection->getStageFlags() == stage_flag)
			return reflection;

	return {};
}

ShaderStageFlags ProgramReflection::getShaderStageFlags() const VERA_NOEXCEPT
{
	return getImpl(this).shaderStageFlags;
}

array_view<ReflectionEntryPoint> ProgramReflection::enumerateEntryPoints() const VERA_NOEXCEPT
{
	return getImpl(this).entryPoints;
}

const ReflectionEntryPoint* ProgramReflection::getEntryPoint(ShaderStageFlags stage_flag) const VERA_NOEXCEPT
{
	if (stage_flag.count() != 1)
		throw Exception("getEntryPoint requires a single ShaderStageFlagBits");

	for (const auto& entry_point : getImpl(this).entryPoints)
		if (entry_point.stageFlags == stage_flag)
			return &entry_point;
	
	return nullptr;
}

hash_t ProgramReflection::hash() const VERA_NOEXCEPT
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END
