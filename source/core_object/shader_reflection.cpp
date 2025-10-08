#include "../../include/vera/core/shader_reflection.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"

VERA_NAMESPACE_BEGIN

static bool has_same_device(std::span<obj<Shader>> shaders)
{
	auto& device = CoreObject::getImpl(shaders[0]).device;

	for (auto& shader : shaders) {
		if (CoreObject::getImpl(shader).device != device)
			return false;
	}
	return true;
}

static bool check_shader_stages(std::span<obj<Shader>> shaders)
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

static ReflectionDesc* copy_reflection(const ReflectionDesc* desc)
{
	VERA_ASSERT(desc && desc->type != ReflectionType::Unknown);

	auto* new_desc = new ReflectionDesc(*desc);
	new_desc->name           = desc->name;
	new_desc->type           = desc->type;

	switch (desc->type) {

	}

	return new_desc;
}

// Checks shader reflection descriptors are identical except shader stage flags and name
static bool check_same_reflection(const ReflectionDesc* a, const ReflectionDesc* b)
{
	if (a == b)
		return true;
	if (a->type != b->type)
		return false;

	switch (a->type) {
	case ReflectionType::Primitive:{
		const auto& desc_a = static_cast<const ReflectionPrimitiveDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionPrimitiveDesc&>(*b);
		return desc_a.primitiveType == desc_b.primitiveType && desc_a.offset == desc_b.offset;
	}
	case ReflectionType::Array: {
		const auto& desc_a = static_cast<const ReflectionArrayDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionArrayDesc&>(*b);
		return 
			desc_a.stride == desc_b.stride &&
			desc_a.elementCount == desc_b.elementCount &&
			check_same_reflection(desc_a.element, desc_b.element) &&
			desc_a.offset == desc_b.offset;
	}
	case ReflectionType::Struct: {
		const auto& desc_a = static_cast<const ReflectionStructDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionStructDesc&>(*b);
		
		if (desc_a.memberCount != desc_b.memberCount || desc_a.offset != desc_b.offset)
			return false;

		for (size_t i = 0; i < desc_a.memberCount; ++i)
			if (!check_same_reflection(desc_a.members[i], desc_b.members[i]))
				return false;
		
		return true;
	}
	case ReflectionType::Resource: {
		const auto& desc_a = static_cast<const ReflectionResourceDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionResourceDesc&>(*b);
		return
			desc_a.resourceLayout == desc_b.resourceLayout &&
			desc_a.resourceType == desc_b.resourceType &&
			desc_a.set == desc_b.set&&
			desc_a.binding == desc_b.binding;
	}
	case ReflectionType::ResourceBlock: {
		const auto& desc_a = static_cast<const ReflectionResourceBlockDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionResourceBlockDesc&>(*b);
		
		if (desc_a.resourceLayout == desc_b.resourceLayout ||
			desc_a.sizeInByte != desc_b.sizeInByte ||
			desc_a.resourceType != desc_b.resourceType ||
			desc_a.set != desc_b.set ||
			desc_a.binding != desc_b.binding ||
			desc_a.memberCount != desc_b.memberCount)
			return true;

		for (size_t i = 0; i < desc_a.memberCount; ++i)
			if (!check_same_reflection(desc_a.members[i], desc_b.members[i]))
				return false;

		return true;
	}
	case ReflectionType::PushConstant: {
		const auto& desc_a = static_cast<const ReflectionPushConstantDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionPushConstantDesc&>(*b);

		if (desc_a.sizeInByte != desc_b.sizeInByte ||
			desc_a.memberCount != desc_b.memberCount)
			return false;
		
		for (size_t i = 0; i < desc_a.memberCount; ++i)
			if (!check_same_reflection(desc_a.members[i], desc_b.members[i]))
				return false;

		return true;
	}
	case ReflectionType::ResourceArray: {
		const auto& desc_a = static_cast<const ReflectionResourceArrayDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionResourceArrayDesc&>(*b);

		return
			desc_a.resourceLayout == desc_b.resourceLayout &&
			desc_a.resourceType == desc_b.resourceType &&
			desc_a.set == desc_b.set &&
			desc_a.binding == desc_b.binding &&
			desc_a.elementCount == desc_b.elementCount &&
			check_same_reflection(desc_a.element, desc_b.element);
	}
	}
}

static std::vector<ReflectionDesc*> combine_all_reflections(std::span<obj<Shader>> shaders)
{
	static const auto sort_by_name = [](const ReflectionDesc* a, const ReflectionDesc* b) {
		return strcmp(a->name, b->name) < 0;
	};

	static const auto sort_by_set_binding = [](const ReflectionRootDesc* a, const ReflectionRootDesc* b) {

	};
	
	std::vector<ReflectionDesc*> result;
	
	for (const auto& shader : shaders)
		result.insert(result.cend(), VERA_SPAN(CoreObject::getImpl(shader).reflections));

	for (size_t i = 0; i < result.size(); ++i) {
		for (size_t j = i + 1; j < result.size(); ++j) {
			if (strcmp(result[i]->name, result[j]->name) != 0) continue;

			if (!check_same_reflection(result[i], result[j]))
				throw Exception("resources with same names in different shader stages are not compatible");
		
			// create new resource layout with combined shader stage flags


		}
	}

	return result;
}

obj<ShaderReflection> ShaderReflection::create(std::vector<obj<Shader>> shaders)
{
	static const auto sort_by_stage = [](const_ref<Shader> a, const_ref<Shader> b) {
		return
			static_cast<uint32_t>(a->getShaderStageFlags()) <
			static_cast<uint32_t>(b->getShaderStageFlags());
	};

	if (!has_same_device(shaders))
		throw Exception("can't reflect shaders from another devices");
	if (!check_shader_stages(shaders))
		throw Exception("can't reflect shaders with duplicate shader stages");

	std::sort(VERA_SPAN(shaders), sort_by_stage);

	auto  obj       = createNewCoreObject<ShaderReflection>();
	auto& impl      = getImpl(obj);
	auto  all_descs = combine_all_reflections(shaders);

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

obj<Device> ShaderReflection::getDevice()
{
	return getImpl(this).device;
}

bool ShaderReflection::isCompatible(obj<Pipeline>& pipeline) const
{
	// TODO: implement

	return false;
}

VERA_NAMESPACE_END