#include "../../include/vera/core/descriptor_set_layout.h"
#include "../impl/device_impl.h"
#include "../impl/descriptor_set_layout_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/util/array_view.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

static uint32_t get_max_resource_count(const DeviceImpl& impl, DescriptorType resource_type)
{
	const auto& indexing_props = impl.descriptorIndexingProperties;

	switch (resource_type) {
	case DescriptorType::Sampler:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers;
	case DescriptorType::CombinedImageSampler:
		return std::min(
			indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers,
			indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages);
	case DescriptorType::SampledImage:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages;
	case DescriptorType::StorageImage:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindStorageImages;
	case DescriptorType::UniformTexelBuffer:
	case DescriptorType::UniformBuffer:
	case DescriptorType::UniformBufferDynamic:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
	case DescriptorType::StorageTexelBuffer:
	case DescriptorType::StorageBuffer:
	case DescriptorType::StorageBufferDynamic:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
	case DescriptorType::InputAttachment:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindInputAttachments;
	}

	VERA_ASSERT_MSG(false, "invalid resource type");
	return {};
}

static bool check_contiguous(array_view<DescriptorSetLayoutBinding> ordered_bindings)
{
	if (ordered_bindings.front().binding != 0)
		return false;

	for (size_t i = 1; i < ordered_bindings.size(); ++i)
		if (ordered_bindings[i].binding != ordered_bindings[i - 1].binding + 1)
			return false;

	return true;
}

static hash_t hash_descriptor_set_layout(const DescriptorSetLayoutCreateInfo& info)
{
	hash_t seed = 0;

	hash_combine(seed, static_cast<uint32_t>(info.flags));

	hash_combine(seed, info.bindings.size());
	for (const auto& binding : info.bindings) {
		hash_t local_seed = 0;

		hash_combine(local_seed, static_cast<uint32_t>(binding.flags));
		hash_combine(local_seed, binding.binding);
		hash_combine(local_seed, binding.descriptorType);
		hash_combine(local_seed, binding.descriptorCount);
		hash_combine(local_seed, static_cast<uint32_t>(binding.stageFlags));

		hash_unordered(seed, local_seed);
	}

	return seed;
}

const vk::DescriptorSetLayout& get_vk_descriptor_set_layout(const_ref<DescriptorSetLayout> descriptor_set_layout)
{
	return CoreObject::getImpl(descriptor_set_layout).descriptorSetLayout;
}

vk::DescriptorSetLayout& get_vk_descriptor_set_layout(ref<DescriptorSetLayout> descriptor_set_layout)
{
	return CoreObject::getImpl(descriptor_set_layout).descriptorSetLayout;
}

obj<DescriptorSetLayout> DescriptorSetLayout::create(obj<Device> device, const DescriptorSetLayoutCreateInfo& info)
{
	VERA_ASSERT_MSG(device, "device is null");
	VERA_ASSERT_MSG(!info.bindings.empty(), "descriptor set layout must have at least one binding");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_descriptor_set_layout(info);

	if (auto it = device_impl.descriptorSetLayoutCacheMap.find(hash_value);
		it != device_impl.descriptorSetLayoutCacheMap.end()) {
		return unsafe_obj_cast<DescriptorSetLayout>(it->second);
	}

	auto  obj  = createNewCoreObject<DescriptorSetLayout>();
	auto& impl = getImpl(obj);

	impl.bindings.assign(VERA_SPAN(info.bindings));

	std::sort(VERA_SPAN(impl.bindings),
		[](const auto& lhs, const auto& rhs) {
			return lhs.binding < rhs.binding;
		});

	VERA_ASSERT_MSG(check_contiguous(impl.bindings), "resource layout bindings can not be sparse");

	static_vector<vk::DescriptorSetLayoutBinding, 32> bindings;
	static_vector<vk::DescriptorBindingFlags, 32>     binding_flags;

	for (const auto& binding : impl.bindings) {
		auto& vk_binding = bindings.emplace_back();
		vk_binding.binding            = binding.binding;
		vk_binding.descriptorType     = to_vk_descriptor_type(binding.descriptorType);
		vk_binding.descriptorCount    = binding.descriptorCount;
		vk_binding.stageFlags         = to_vk_shader_stage_flags(binding.stageFlags);
		vk_binding.pImmutableSamplers = nullptr;

		if (vk_binding.descriptorCount == UINT32_MAX)
			vk_binding.descriptorCount = get_max_resource_count(device_impl, binding.descriptorType);

		binding_flags.push_back(to_vk_descriptor_binding_flags(binding.flags));
	}

	vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info;
	binding_flags_info.bindingCount  = static_cast<uint32_t>(binding_flags.size());
	binding_flags_info.pBindingFlags = binding_flags.data();

	vk::DescriptorSetLayoutCreateInfo desc_info;
	desc_info.flags        = to_vk_descriptor_set_layout_create_flags(info.flags);
	desc_info.bindingCount = static_cast<uint32_t>(bindings.size());
	desc_info.pBindings    = bindings.data();
	desc_info.pNext        = &binding_flags_info;
	
	impl.device              = std::move(device);
	impl.descriptorSetLayout = device_impl.device.createDescriptorSetLayout(desc_info);
	impl.hashValue           = hash_value;
	
	device_impl.descriptorSetLayoutCacheMap.insert({ hash_value, obj });
	
	return obj;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
	
	device_impl.descriptorSetLayoutCacheMap.erase(impl.hashValue);
	device_impl.device.destroy(impl.descriptorSetLayout);

	destroyObjectImpl(this);
}

obj<Device> DescriptorSetLayout::getDevice()
{
	return getImpl(this).device;
}

const std::vector<DescriptorSetLayoutBinding>& DescriptorSetLayout::getBindings() const
{
	return getImpl(this).bindings;
}

hash_t DescriptorSetLayout::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END