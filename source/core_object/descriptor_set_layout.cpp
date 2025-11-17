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
	const auto& indexing_props = impl.vkDescriptorIndexingProperties;

	switch (resource_type) {
	case DescriptorType::Sampler:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers;
	case DescriptorType::CombinedTextureSampler:
		return std::min(
			indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers,
			indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages);
	case DescriptorType::SampledTexture:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages;
	case DescriptorType::StorageTexture:
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

const vk::DescriptorSetLayout& get_vk_descriptor_set_layout(const_ref<DescriptorSetLayout> set_layout) VERA_NOEXCEPT
{
	return CoreObject::getImpl(set_layout).vkDescriptorSetLayout;
}

vk::DescriptorSetLayout& get_vk_descriptor_set_layout(ref<DescriptorSetLayout> set_layout) VERA_NOEXCEPT
{
	return CoreObject::getImpl(set_layout).vkDescriptorSetLayout;
}

obj<DescriptorSetLayout> DescriptorSetLayout::create(obj<Device> device, const DescriptorSetLayoutCreateInfo& info)
{
	VERA_ASSERT_MSG(device, "device is null");
	VERA_ASSERT_MSG(!info.bindings.empty(), "descriptor set layout must have at least one binding");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_descriptor_set_layout(info);

	if (auto it = device_impl.descriptorSetLayoutCache.find(hash_value);
		it != device_impl.descriptorSetLayoutCache.end()) {
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

	for (auto& binding : impl.bindings) {
		auto& vk_binding = bindings.emplace_back();
		vk_binding.binding            = binding.binding;
		vk_binding.descriptorType     = to_vk_descriptor_type(binding.descriptorType);
		vk_binding.descriptorCount    = binding.descriptorCount;
		vk_binding.stageFlags         = to_vk_shader_stage_flags(binding.stageFlags);
		vk_binding.pImmutableSamplers = nullptr;

		if (vk_binding.descriptorCount == UINT32_MAX)
			vk_binding.descriptorCount = get_max_resource_count(device_impl, binding.descriptorType);

		binding_flags.push_back(to_vk_descriptor_binding_flags(binding.flags));

		impl.bindingMap[binding.binding] = &binding;
	}

	vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info;
	binding_flags_info.bindingCount  = static_cast<uint32_t>(binding_flags.size());
	binding_flags_info.pBindingFlags = binding_flags.data();

	vk::DescriptorSetLayoutCreateInfo desc_info;
	desc_info.flags        = to_vk_descriptor_set_layout_create_flags(info.flags);
	desc_info.bindingCount = static_cast<uint32_t>(bindings.size());
	desc_info.pBindings    = bindings.data();
	desc_info.pNext        = &binding_flags_info;
	
	impl.device                = std::move(device);
	impl.vkDescriptorSetLayout = device_impl.vkDevice.createDescriptorSetLayout(desc_info);
	impl.hashValue             = hash_value;
	
	device_impl.registerDescriptorSetLayout(hash_value, obj);
	
	return obj;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
	
	device_impl.unregisterDescriptorSetLayout(impl.hashValue);
	device_impl.vkDevice.destroy(impl.vkDescriptorSetLayout);

	destroyObjectImpl(this);
}

obj<Device> DescriptorSetLayout::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

uint32_t DescriptorSetLayout::getBindingCount() const VERA_NOEXCEPT
{
	return static_cast<uint32_t>(getImpl(this).bindings.size());
}

const DescriptorSetLayoutBinding& DescriptorSetLayout::getBinding(uint32_t binding) const VERA_NOEXCEPT
{
	return *getImpl(this).bindingMap.at(binding);
}

const std::vector<DescriptorSetLayoutBinding>& DescriptorSetLayout::getBindings() const VERA_NOEXCEPT
{
	return getImpl(this).bindings;
}

hash_t DescriptorSetLayout::hash() const VERA_NOEXCEPT
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END