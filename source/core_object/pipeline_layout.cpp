#include "../../include/vera/core/pipeline_layout.h"
#include "../impl/device_impl.h"
#include "../impl/pipeline_impl.h"
#include "../impl/pipeline_layout_impl.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/descriptor_set_layout_impl.h"

#include "../../include/vera/util/hash.h"
#include "../../include/vera/util/static_vector.h"

#define MAX_SHADER_STAGE_COUNT 16
#define MAX_SET_COUNT 128
#define MAX_PC_RANGE_COUNT 16

#define UNSIZED_ARRAY_BINDING_FLAGS \
	DescriptorSetLayoutBindingFlagBits::UpdateAfterBind | \
	DescriptorSetLayoutBindingFlagBits::PartiallyBound | \
	DescriptorSetLayoutBindingFlagBits::VariableDescriptorCount

VERA_NAMESPACE_BEGIN

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

static void create_resource_layout(PipelineLayoutImpl& impl, const ReflectionRootNode* root_node)
{
	DescriptorSetLayoutCreateInfo layout_info;

	uint32_t set_count = root_node->getSetCount();

	for (uint32_t set_id = 0; set_id < set_count; ++set_id) {
		const auto& set_range = root_node->enumerateDescriptorSet(set_id);

		if (set_range.empty())
			throw Exception("set={} is empty, set must be contiguous", set_id);
		
		layout_info.bindings.clear();
		layout_info.flags = DescriptorSetLayoutCreateFlags{};

		for (const auto* desc_node : set_range) {
			auto& layout_binding = layout_info.bindings.emplace_back();
			layout_binding.binding        = desc_node->binding;
			layout_binding.descriptorType = desc_node->descriptorType;
			layout_binding.stageFlags     = desc_node->stageFlags;

			if (desc_node->type == ReflectionType::DescriptorArray) {
				uint32_t elem_count = desc_node->getElementCount();

				if (elem_count == UINT32_MAX) {
					layout_binding.flags           = UNSIZED_ARRAY_BINDING_FLAGS;
					layout_binding.descriptorCount = UINT32_MAX;
					layout_info.flags              = DescriptorSetLayoutCreateFlagBits::UpdateAfterBindPool;
				} else {
					layout_binding.descriptorCount = elem_count;
				}
			} else {
				layout_binding.descriptorCount = 1;
			}
		}

		auto new_layout = DescriptorSetLayout::create(impl.device, layout_info);
		impl.descriptorSetLayouts.push_back(std::move(new_layout));
	}
}

static void create_pc_info(PipelineLayoutImpl& impl, const ReflectionRootNode* root_node)
{
	for (const auto* pc_node : root_node->enumeratePushConstant())
		impl.pushConstantRanges.emplace_back(pc_node->pushConstantRange);
}

static void create_pipeline_layout(const DeviceImpl & device_impl, PipelineLayoutImpl& impl)
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

	impl.pipelineLayout = device_impl.device.createPipelineLayout(pipeline_layout_info);
}

static bool check_layout_device(ref<Device> device, array_view<obj<DescriptorSetLayout>> layouts)
{
	if (layouts.empty()) return false;

	for (const auto layout : layouts)
		if (!layout || CoreObject::getImpl(layout).device != device)
			return false;
	return true;
}

static hash_t hash_pipeline_layout_with_shaders(const array_view<obj<Shader>>& shaders)
{
	hash_t seed = 0;

	hash_combine(seed, shaders.size());
	for (const auto& shader : shaders)
		hash_combine(seed, shader->hash());

	return seed;
}

static hash_t hash_pipeline_layout(const PipelineLayoutCreateInfo& info)
{
	hash_t seed = 0;

	hash_combine(seed, info.descriptorSetLayouts.size());
	for (const auto& layout : info.descriptorSetLayouts)
		hash_combine(seed, layout->hash());
		
	hash_combine(seed, info.pushConstantRanges.size());
	for (const auto& range : info.pushConstantRanges) {
		hash_combine(seed, range.offset);
		hash_combine(seed, range.size);
		hash_combine(seed, static_cast<uint32_t>(range.stageFlags));
	}

	hash_combine(seed, info.pipelineBindPoint);

	return seed;
}

static hash_t hash_pipeline_layout(const PipelineLayoutImpl& impl)
{
	hash_t seed = 0;

	hash_combine(seed, impl.descriptorSetLayouts.size());
	for (const auto& layout : impl.descriptorSetLayouts)
		hash_combine(seed, layout->hash());
		
	hash_combine(seed, impl.pushConstantRanges.size());
	for (const auto& range : impl.pushConstantRanges) {
		hash_combine(seed, range.offset);
		hash_combine(seed, range.size);
		hash_combine(seed, static_cast<uint32_t>(range.stageFlags));
	}

	return seed;
}

const vk::PipelineLayout& get_vk_pipeline_layout(const_ref<PipelineLayout> pipeline_layout) VERA_NOEXCEPT
{
	return CoreObject::getImpl(pipeline_layout).pipelineLayout;
}

vk::PipelineLayout& get_vk_pipeline_layout(ref<PipelineLayout> pipeline_layout) VERA_NOEXCEPT
{
	return CoreObject::getImpl(pipeline_layout).pipelineLayout;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, array_view<obj<Shader>> shaders)
{
	VERA_ASSERT_MSG(device, "device is null");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_pipeline_layout_with_shaders(shaders);

	if (auto it = device_impl.pipelineLayoutCacheMapWithShader.find(hash_value);
		it != device_impl.pipelineLayoutCacheMapWithShader.end())
		return unsafe_obj_cast<PipelineLayout>(it->second);

	auto  obj  = createNewCoreObject<PipelineLayout>();
	auto& impl = getImpl(obj);

	impl.device            = std::move(device);
	impl.shaderHashValue   = hash_value;

	for (const auto& shader : shaders)
		impl.shaders.push_back(shader);

	impl.shaderReflection = ShaderReflection::create(array_view{
		reinterpret_cast<const_ref<Shader>*>(impl.shaders.data()),
		impl.shaders.size()
	});

	auto* root_node = get_reflection_root_node(impl.shaderReflection);

	create_resource_layout(impl, root_node);
	create_pc_info(impl, root_node);
	create_pipeline_layout(device_impl, impl);

	impl.hashValue = hash_pipeline_layout(impl);

	device_impl.registerPipelineLayout(impl.hashValue, obj);
	device_impl.registerPipelineLayoutWithShaders(impl.shaderHashValue, obj);

	return obj;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, const PipelineLayoutCreateInfo& info)
{
	VERA_ASSERT_MSG(device, "device is null");
	VERA_ASSERT_MSG(check_layout_device(device, info.descriptorSetLayouts), "layout device mismatch");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_pipeline_layout(info);

	if (auto it = device_impl.pipelineLayoutCacheMap.find(hash_value);
		it != device_impl.pipelineLayoutCacheMap.end())
		return unsafe_obj_cast<PipelineLayout>(it->second);

	auto  obj  = createNewCoreObject<PipelineLayout>();
	auto& impl = getImpl(obj);

	impl.descriptorSetLayouts.assign(info.descriptorSetLayouts.begin(), info.descriptorSetLayouts.end());
	impl.pushConstantRanges.assign(info.pushConstantRanges.begin(), info.pushConstantRanges.end());
	
	// TODO: fill reflection datas

	impl.device            = std::move(device);
	impl.shaderReflection  = {};
	impl.hashValue         = hash_value;
	impl.shaderHashValue   = 0;

	create_pipeline_layout(device_impl, impl);
	
	device_impl.registerPipelineLayout(hash_value, obj);

	return obj;
}

PipelineLayout::~PipelineLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterPipelineLayout(impl.hashValue);
	device_impl.unregisterPipelineLayoutWithShaders(impl.shaderHashValue);
	device_impl.device.destroy(impl.pipelineLayout);

	destroyObjectImpl(this);
}

obj<Device> PipelineLayout::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

obj<DescriptorSetLayout> PipelineLayout::getDescriptorSetLayout(uint32_t set) VERA_NOEXCEPT
{
	return getImpl(this).descriptorSetLayouts[set];
}

obj<ShaderReflection> PipelineLayout::getShaderReflection() VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	if (!impl.shaderReflection) {
		if (impl.shaders.empty())
			throw Exception("pipeline layout has no shaders");
	}

	return impl.shaderReflection;
}

array_view<ref<DescriptorSetLayout>> PipelineLayout::getDescriptorSetLayouts() const VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	return { 
		reinterpret_cast<const ref<DescriptorSetLayout>*>(impl.descriptorSetLayouts.data()),
		impl.descriptorSetLayouts.size()
	};
}

uint32_t PipelineLayout::getDescriptorSetLayoutCount() const VERA_NOEXCEPT
{
	return static_cast<uint32_t>(getImpl(this).descriptorSetLayouts.size());
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
