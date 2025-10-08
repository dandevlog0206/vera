#include "../../include/vera/core/resource_layout.h"
#include "../impl/device_impl.h"
#include "../impl/resource_layout_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

static bool check_contiguous(array_view<ResourceLayoutBinding> ordered_bindings)
{
	if (ordered_bindings.front().binding != 0)
		return false;

	for (size_t i = 1; i < ordered_bindings.size(); ++i)
		if (ordered_bindings[i].binding != ordered_bindings[i - 1].binding + 1)
			return false;

	return true;
}

static hash_t hash_resource_set_layout(const ResourceLayoutCreateInfo& info)
{
	hash_t seed = 0;

	hash_combine(seed, static_cast<uint32_t>(info.flags));

	hash_combine(seed, info.bindings.size());
	for (const auto& binding : info.bindings) {
		hash_t local_seed = 0;

		hash_combine(local_seed, static_cast<uint32_t>(binding.flags));
		hash_combine(local_seed, binding.binding);
		hash_combine(local_seed, binding.resourceType);
		hash_combine(local_seed, binding.resourceCount);
		hash_combine(local_seed, static_cast<uint32_t>(binding.stageFlags));

		hash_unordered(seed, local_seed);
	}

	return seed;
}

const vk::DescriptorSetLayout& get_vk_descriptor_set_layout(const_ref<ResourceLayout> resource_layout)
{
	return CoreObject::getImpl(resource_layout).descriptorSetLayout;
}

vk::DescriptorSetLayout& get_vk_descriptor_set_layout(ref<ResourceLayout> resource_layout)
{
	return CoreObject::getImpl(resource_layout).descriptorSetLayout;
}

obj<ResourceLayout> ResourceLayout::create(obj<Device> device, const ResourceLayoutCreateInfo& info)
{
	VERA_ASSERT_MSG(!info.bindings.empty(), "resource layout must have at least one binding");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_resource_set_layout(info);

	if (auto it = device_impl.resourceLayoutMap.find(hash_value);
		it != device_impl.resourceLayoutMap.end()) {
		return unsafe_obj_cast<ResourceLayout>(it->second);
	}

	auto  obj  = createNewCoreObject<ResourceLayout>();
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
		vk_binding.descriptorType     = to_vk_descriptor_type(binding.resourceType);
		vk_binding.descriptorCount    = binding.resourceCount;
		vk_binding.stageFlags         = to_vk_shader_stage_flags(binding.stageFlags);
		vk_binding.pImmutableSamplers = nullptr;

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
	
	device_impl.resourceLayoutMap[hash_value] = obj;
	
	return obj;
}

ResourceLayout::~ResourceLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
	
	device_impl.device.destroy(impl.descriptorSetLayout);

	destroyObjectImpl(this);
}

obj<Device> ResourceLayout::getDevice()
{
	return getImpl(this).device;
}

const std::vector<ResourceLayoutBinding>& ResourceLayout::getBindings() const
{
	return getImpl(this).bindings;
}

hash_t ResourceLayout::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END