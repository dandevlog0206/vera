#include "../../include/vera/core/resource_layout.h"
#include "../impl/device_impl.h"
#include "../impl/resource_layout_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/util/hash.h"

VERA_NAMESPACE_BEGIN

static size_t hash_resource_binding(const std::vector<ResourceLayoutBinding>& bindings)
{
	size_t seed = 0;

	hash_combine(seed, bindings.size());
	for (const auto& binding : bindings) {
		hash_combine(seed, binding.binding);
		hash_combine(seed, binding.resourceType);
		hash_combine(seed, binding.resourceCount);
		hash_combine(seed, static_cast<size_t>(binding.stageFlags));
	}

	return seed;
}

vk::DescriptorSetLayout get_descriptor_set_layout(const ref<ResourceLayout>& resource_layout)
{
	return CoreObject::getImpl(resource_layout).layout;
}

vk::DescriptorSetLayout& get_descriptor_set_layout(ref<ResourceLayout>& resource_layout)
{
	return CoreObject::getImpl(resource_layout).layout;
}

ref<ResourceLayout> ResourceLayout::create(ref<Device> device, const std::vector<ResourceLayoutBinding>& bindings)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_resource_binding(bindings);

	if (auto it = device_impl.resourceLayoutMap.find(hash_value);
		it != device_impl.resourceLayoutMap.end()) {
		return it->second;
	}

	auto  obj  = createNewObject<ResourceLayout>();
	auto& impl = getImpl(obj);

	std::vector<vk::DescriptorSetLayoutBinding> vk_bindings;
	
	vk_bindings.reserve(bindings.size());
	for (const auto& binding : bindings) {
		vk::DescriptorSetLayoutBinding vk_binding;
		vk_binding.binding            = binding.binding;
		vk_binding.descriptorType     = to_vk_descriptor_type(binding.resourceType);
		vk_binding.descriptorCount    = binding.resourceCount;
		vk_binding.stageFlags         = to_vk_shader_stage_flags(binding.stageFlags);
		vk_binding.pImmutableSamplers = nullptr;

		vk_bindings.push_back(vk_binding);
	}

	vk::DescriptorSetLayoutCreateInfo desc_info;
	desc_info.bindingCount = static_cast<uint32_t>(vk_bindings.size());
	desc_info.pBindings    = vk_bindings.data();
	
	impl.device    = std::move(device);
	impl.layout    = device_impl.device.createDescriptorSetLayout(desc_info);
	impl.hashValue = hash_value;
	impl.bindings.assign(bindings.begin(), bindings.end());
	
	return device_impl.resourceLayoutMap[hash_value] = obj;
}

ResourceLayout::~ResourceLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
	
	device_impl.device.destroy(impl.layout);

	destroyObjectImpl(this);
}

const std::vector<ResourceLayoutBinding>& ResourceLayout::getBindings() const
{
	return getImpl(this).bindings;
}

uint64_t ResourceLayout::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END