#include "../../include/vera/core/pipeline_layout.h"
#include "../impl/device_impl.h"
#include "../impl/pipeline_impl.h"
#include "../impl/pipeline_layout_impl.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/descriptor_set_layout_impl.h"

#include "../../include/vera/util/hash.h"
#include "../../include/vera/util/static_vector.h"

#define UNSIZED_ARRAY_BINDING_FLAGS \
	DescriptorSetLayoutBindingFlagBits::UpdateAfterBind | \
	DescriptorSetLayoutBindingFlagBits::PartiallyBound | \
	DescriptorSetLayoutBindingFlagBits::VariableDescriptorCount

VERA_NAMESPACE_BEGIN

enum 
{
	MAX_SHADER_STAGE_COUNT = 16,
	MAX_SET_COUNT          = 128,
	MAX_PC_RANGE_COUNT     = 16
};

typedef static_vector<ShaderStageFlags, MAX_SHADER_STAGE_COUNT> PerStageShaderStageFlagsArray;
typedef static_vector<PushConstantRange, MAX_SHADER_STAGE_COUNT> PerStagePushConstantArray;

static bool operator==(const PushConstantRange& lhs, const PushConstantRange& rhs) VERA_NOEXCEPT
{
	return
		lhs.offset == rhs.offset &&
		lhs.size == rhs.size &&
		lhs.stageFlags == rhs.stageFlags;
}

static PipelineBindPoint get_pipeline_bind_point(ShaderStageFlags stage_flags)
{
	VERA_ASSERT_MSG(stage_flags.count() == 1, "stage flags must have exactly one bit set");

	switch ((ShaderStageFlagBits)stage_flags.mask()) {
	case ShaderStageFlagBits::Vertex:
	case ShaderStageFlagBits::TessellationControl:
	case ShaderStageFlagBits::TessellationEvaluation:
	case ShaderStageFlagBits::Geometry:
	case ShaderStageFlagBits::Fragment:
		return PipelineBindPoint::Graphics;
	case ShaderStageFlagBits::Compute:
		return PipelineBindPoint::Compute;
	}

	VERA_ASSERT_MSG(false, "invalid shader stage for pipeline bind point");
	return {};
}

static bool push_constant_ranges_overlap(
	const PushConstantRange& a,
	const PushConstantRange& b
) VERA_NOEXCEPT {
	if (a.offset >= b.offset + b.size) return false;
	if (b.offset >= a.offset + a.size) return false;
	return true;
}

static void insert_descriptor_binding_info(
	DescriptorSetLayoutCreateInfo&           layout_info,
	const ReflectionDescriptorBinding*       binding_info
) {
	DescriptorSetLayoutBinding* p_binding = nullptr;

	for (auto& binding : layout_info.bindings) {
		if (binding.binding == binding_info->binding) {
			p_binding = &binding;
			break;
		}
	}

	if (!p_binding) {
		auto& new_binding = layout_info.bindings.emplace_back();
		new_binding.flags           = {};
		new_binding.binding         = binding_info->binding;
		new_binding.descriptorType  = binding_info->descriptorType;
		new_binding.descriptorCount = binding_info->elementCount;
		new_binding.stageFlags      = binding_info->stageFlags;

		if (binding_info->arrayTraits.isUnsized()) {
			layout_info.flags           = DescriptorSetLayoutCreateFlagBits::UpdateAfterBindPool;
			new_binding.flags           = UNSIZED_ARRAY_BINDING_FLAGS;
			new_binding.descriptorCount = UINT32_MAX;
		}
	} else {
		if (p_binding->descriptorType != binding_info->descriptorType)
			throw Exception("incompatible descriptor type at binding={}", binding_info->binding);

		p_binding->descriptorCount = std::max(
			p_binding->descriptorCount,
			binding_info->elementCount
		);
		p_binding->stageFlags     |= binding_info->stageFlags;

		if (binding_info->arrayTraits.isUnsized()) {
			layout_info.flags          = DescriptorSetLayoutCreateFlagBits::UpdateAfterBindPool;
			p_binding->flags           = UNSIZED_ARRAY_BINDING_FLAGS;
			p_binding->descriptorCount = UINT32_MAX;
		}
	}
}

static void create_pipeline_layout(const DeviceImpl& device_impl, PipelineLayoutImpl& impl)
{
	static_vector<vk::DescriptorSetLayout, MAX_SET_COUNT>    vk_layouts;
	static_vector<vk::PushConstantRange, MAX_PC_RANGE_COUNT> vk_pc_ranges;

	for (const auto& layout : impl.descriptorSetLayouts)
		vk_layouts.push_back(get_vk_descriptor_set_layout(layout));
	for (const auto& range : impl.pushConstantRanges)
		vk_pc_ranges.push_back(get_vk_push_constant_range(range));
	
	vk::PipelineLayoutCreateInfo pipeline_layout_info;
	pipeline_layout_info.setLayoutCount         = static_cast<uint32_t>(vk_layouts.size());
	pipeline_layout_info.pSetLayouts            = vk_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(vk_pc_ranges.size());
	pipeline_layout_info.pPushConstantRanges    = vk_pc_ranges.data();

	impl.vkPipelineLayout = device_impl.vkDevice.createPipelineLayout(pipeline_layout_info);
}

static bool check_shader_device(ref<Device> device, array_view<const_ref<Shader>> shaders)
{
	if (shaders.empty()) return false;
	for (const auto& shader : shaders)
		if (!shader || CoreObject::getImpl(shader).device != device)
			return false;
	return true;
}

static bool check_shader_reflection_device(
	ref<Device> device,
	array_view<const_ref<ShaderReflection>> shader_reflections
) {
	if (shader_reflections.empty()) return false;
	for (const auto& reflection : shader_reflections)
		if (!reflection || CoreObject::getImpl(reflection).device != device)
			return false;
	return true;
}

static bool check_layout_device(ref<Device> device, array_view<obj<DescriptorSetLayout>> layouts)
{
	if (layouts.empty()) return false;

	for (const auto layout : layouts)
		if (!layout || CoreObject::getImpl(layout).device != device)
			return false;
	return true;
}

static hash_t hash_shaders(
	array_view<const_ref<Shader>> shaders
) {
	hash_t seed = 0;

	for (const auto shader : shaders)
		hash_unordered(seed, shader->hash());

	return seed;
}

static hash_t hash_shader_reflections(
	array_view<const_ref<ShaderReflection>> shader_reflections
) {
	hash_t seed = 1;

	for (const auto& reflection : shader_reflections)
		hash_unordered(seed, reflection->hash());
	
	return seed;
}

static hash_t hash_pipeline_layout(
	array_view<obj<DescriptorSetLayout>> set_layouts,
	array_view<PushConstantRange>        pc_ranges
) {
	hash_t seed = 2;

	hash_combine(seed, set_layouts.size());
	for (const auto& layout : set_layouts)
		hash_unordered(seed, layout->hash());

	hash_combine(seed, pc_ranges.size());
	for (const auto& range : pc_ranges) {
		hash_combine(seed, range.offset);
		hash_combine(seed, range.size);
		hash_combine(seed, static_cast<uint32_t>(range.stageFlags));
	}

	return seed;
}

const vk::PipelineLayout& get_vk_pipeline_layout(const_ref<PipelineLayout> pipeline_layout) VERA_NOEXCEPT
{
	return CoreObject::getImpl(pipeline_layout).vkPipelineLayout;
}

vk::PipelineLayout& get_vk_pipeline_layout(ref<PipelineLayout> pipeline_layout) VERA_NOEXCEPT
{
	return CoreObject::getImpl(pipeline_layout).vkPipelineLayout;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, array_view<const_ref<Shader>> shaders)
{
	if (!device)
		throw Exception("device is null");
	if (!check_shader_device(device, shaders))
		throw Exception("shader device mismatch");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_shaders(shaders);

	if (auto cached_obj = device_impl.findCachedObject<PipelineLayout>(hash_value))
		return cached_obj;

	static_vector<obj<ShaderReflection>, MAX_SHADER_STAGE_COUNT> shader_reflections;

	for (auto shader : shaders)
		shader_reflections.push_back(ShaderReflection::create(device, shader));

	auto  obj  = PipelineLayout::create(device, 
		array_view<const_ref<ShaderReflection>>(
			reinterpret_cast<const const_ref<ShaderReflection>*>(shader_reflections.data()),
			shader_reflections.size()));
	auto& impl = getImpl(obj);

	impl.hashValueByShaders = hash_value;

	return obj;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, array_view<const_ref<ShaderReflection>> shader_reflections)
{
	static const auto sort_by_offset = 
		[](const PushConstantRange& a, const PushConstantRange& b) {
			return a.offset < b.offset;
		};

	if (!device)
		throw Exception("device is null");
	if (!check_shader_reflection_device(device, shader_reflections))
		throw Exception("shader reflection device mismatch");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_shader_reflections(shader_reflections);

	if (auto cached_obj = device_impl.findCachedObject<PipelineLayout>(hash_value))
		return cached_obj;

	auto  obj  = createNewCoreObject<PipelineLayout>();
	auto& impl = getImpl(obj);

	uint32_t set_count = 0;
	for (auto reflection : shader_reflections)
		if (auto bindings = reflection->enumerateDescriptorBindings(); !bindings.empty())
			set_count = std::max(set_count, bindings.back()->set + 1);

	DescriptorSetLayoutCreateInfo layout_info;

	for (uint32_t set_id = 0; set_id < set_count; ++set_id) {
		for (auto reflection : shader_reflections)
			for (const auto* desc_binding : reflection->enumerateDescriptorBindings(set_id))
				insert_descriptor_binding_info(layout_info, desc_binding);

		impl.descriptorSetLayouts.push_back(
			DescriptorSetLayout::create(device, layout_info));

		layout_info.flags = {};
		layout_info.bindings.clear();
	}

	for (auto reflection : shader_reflections) {
		if (const auto* pc_block = reflection->getPushConstantBlock()) {
			auto& pc_range = impl.pushConstantRanges.emplace_back();
			pc_range.offset     = pc_block->offset;
			pc_range.size       = pc_block->size;
			pc_range.stageFlags = pc_block->stageFlags;
		}
	}

	if (!impl.pushConstantRanges.empty()) {
		std::sort(VERA_SPAN(impl.pushConstantRanges), sort_by_offset);

		// Merge overlapping push constant ranges inplace
		auto src_it = impl.pushConstantRanges.begin();
		auto dst_it = impl.pushConstantRanges.begin();

		while (src_it != impl.pushConstantRanges.end()) {
			auto& dst_range = *dst_it;
			auto& src_range = *src_it;

			if (dst_it != src_it && src_range.offset < dst_range.offset + dst_range.size) {
				auto end_offset = std::max(
					dst_range.offset + dst_range.size,
					src_range.offset + src_range.size);

				dst_range.size        = end_offset - dst_range.offset;
				dst_range.stageFlags |= src_range.stageFlags;
			} else {
				if (dst_it != src_it)
					*(++dst_it) = src_range;
				else
					++dst_it;
			}
			++src_it;
		}
	}

	impl.device                 = std::move(device);
	impl.hashValue              = hash_pipeline_layout(impl.descriptorSetLayouts, impl.pushConstantRanges);
	impl.hashValueByReflections = hash_value;
	impl.hashValueByShaders     = 0;

	create_pipeline_layout(device_impl, impl);

	device_impl.registerCachedObject<PipelineLayout>(hash_value, obj);

	return obj;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, const PipelineLayoutCreateInfo& info)
{
	if (!device)
		throw Exception("device is null");
	if (!check_layout_device(device, info.descriptorSetLayouts))
		throw Exception("layout device mismatch");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_pipeline_layout(info.descriptorSetLayouts, info.pushConstantRanges);

	if (auto cached_obj = device_impl.findCachedObject<PipelineLayout>(hash_value))
		return cached_obj;

	auto  obj  = createNewCoreObject<PipelineLayout>();
	auto& impl = getImpl(obj);

	impl.descriptorSetLayouts.assign(info.descriptorSetLayouts.begin(), info.descriptorSetLayouts.end());
	impl.pushConstantRanges.assign(info.pushConstantRanges.begin(), info.pushConstantRanges.end());

	impl.device                 = std::move(device);
	impl.hashValue              = hash_value;
	impl.hashValueByReflections = 0;
	impl.hashValueByShaders     = 0;

	create_pipeline_layout(device_impl, impl);
	
	device_impl.registerCachedObject<PipelineLayout>(hash_value, obj);

	return obj;
}

//obj<PipelineLayout> PipelineLayout::create(
//	obj<Device>                          device,
//	array_view<obj<DescriptorSetLayout>> set_layouts,
//	array_view<PushConstantRange>        pc_ranges)
//{
//	if (!device)
//		throw Exception("device is null");
//	if (!check_layout_device(device, set_layouts))
//		throw Exception("layout device mismatch");
//
//	auto&  device_impl = getImpl(device);
//	hash_t hash_value  = hash_pipeline_layout(set_layouts, pc_ranges);
//
//	if (auto it = device_impl.pipelineLayoutCacheMap.find(hash_value);
//		it != device_impl.pipelineLayoutCacheMap.end())
//		return unsafe_obj_cast<PipelineLayout>(it->second);
//
//	auto  obj  = createNewCoreObject<PipelineLayout>();
//	auto& impl = getImpl(obj);
//
//	impl.descriptorSetLayouts.assign(set_layouts.begin(), set_layouts.end());
//	impl.pushConstantRanges.assign(pc_ranges.begin(), pc_ranges.end());
//
//	impl.device            = std::move(device);
//	impl.shaderReflection  = {};
//	impl.hashValue         = hash_value;
//
//	create_pipeline_layout(device_impl, impl);
//	
//	device_impl.registerPipelineLayout(hash_value, obj);
//
//	return obj;
//}

PipelineLayout::~PipelineLayout() VERA_NOEXCEPT
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterCachedObject<PipelineLayout>(impl.hashValue);
	if (impl.hashValueByShaders)
		device_impl.unregisterCachedObject<PipelineLayout>(impl.hashValueByShaders);
	if (impl.hashValueByReflections)
		device_impl.unregisterCachedObject<PipelineLayout>(impl.hashValueByReflections);

	device_impl.vkDevice.destroy(impl.vkPipelineLayout);

	destroyObjectImpl(this);
}

obj<Device> PipelineLayout::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

uint32_t PipelineLayout::getDescriptorSetLayoutCount() const VERA_NOEXCEPT
{
	return static_cast<uint32_t>(getImpl(this).descriptorSetLayouts.size());
}

obj<DescriptorSetLayout> PipelineLayout::getDescriptorSetLayout(uint32_t set) VERA_NOEXCEPT
{
	return getImpl(this).descriptorSetLayouts[set];
}

array_view<ref<DescriptorSetLayout>> PipelineLayout::enumerateDescriptorSetLayouts() const VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	return { 
		reinterpret_cast<const ref<DescriptorSetLayout>*>(impl.descriptorSetLayouts.data()),
		impl.descriptorSetLayouts.size()
	};
}

array_view<PushConstantRange> PipelineLayout::getPushConstantRanges() const VERA_NOEXCEPT
{
	return getImpl(this).pushConstantRanges;
}

bool PipelineLayout::isCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT
{
	return isDescriptorSetLayoutCompatible(pipeline_layout) && isPushConstantCompatible(pipeline_layout);
}

bool PipelineLayout::isDescriptorSetLayoutCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT
{
	auto& lhs_impl = getImpl(this);
	auto& rhs_impl = getImpl(pipeline_layout);

	// Compare each descriptor set layout. Since descriptor set layout is unique,
	// we can compare it directly.
	return std::equal(
		lhs_impl.descriptorSetLayouts.begin(),
		lhs_impl.descriptorSetLayouts.end(),
		rhs_impl.descriptorSetLayouts.begin()
	);
}

bool PipelineLayout::isPushConstantCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT
{
	auto& lhs_impl = getImpl(this);
	auto& rhs_impl = getImpl(pipeline_layout);

	return std::equal(
		lhs_impl.pushConstantRanges.begin(),
		lhs_impl.pushConstantRanges.end(),
		rhs_impl.pushConstantRanges.begin()
	);
}

size_t PipelineLayout::hash() const VERA_NOEXCEPT
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END
