#include "../../include/vera/core/resource_binding_pool.h"
#include "../impl/resource_binding_pool_impl.h"
#include "../impl/resource_binding_impl.h"
#include "../impl/resource_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/resource_layout.h"
#include "../../include/vera/core/resource_binding.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/util/hash.h"

VERA_NAMESPACE_BEGIN

static hash_t hash_request(const_ref<ResourceLayout> layout, array_view<ResourceBindingInfo> binding_infos)
{
	hash_t hash_value = 0;

	hash_combine(hash_value, layout->hash());

	hash_combine(hash_value, binding_infos.size());
	for (const auto& info : binding_infos)
		hash_unordered(hash_value, info.hash());

	return hash_value;
}

static void fill_binding_states(
	ResourceBindingImpl&      binding_impl,
	const ResourceLayoutImpl& layout_impl,
	uint32_t                  variable_binding_count
) {
	binding_impl.bindingStates.resize(layout_impl.bindings.size());

	for (const auto& layout_binding : layout_impl.bindings) {
		auto& array_desc = binding_impl.bindingStates[layout_binding.binding];

		if (layout_binding.flags.has(ResourceLayoutBindingFlagBits::VariableBindingCount)) {
			array_desc.bindingDescs.resize(variable_binding_count);
			break;
		}

		array_desc.bindingDescs.resize(layout_binding.resourceCount);
	}
}

static void fill_binding_states_from_infos(
	ResourceBindingImpl&            binding_impl,
	const ResourceLayoutImpl&       layout_impl,
	array_view<ResourceBindingInfo> binding_infos
) {
	hash_t hash_value = 0;
	size_t n          = 0;

	binding_impl.bindingStates.resize(layout_impl.bindings.size());

	for (const auto& layout_binding : layout_impl.bindings) {
		auto& array_desc = binding_impl.bindingStates[layout_binding.binding];
		
		if (layout_binding.flags.has(ResourceLayoutBindingFlagBits::VariableBindingCount)) {
			binding_impl.arrayElementCount = static_cast<uint32_t>(binding_infos.size() - n);

			array_desc.bindingDescs.resize(binding_impl.arrayElementCount);

			for (auto& desc : array_desc.bindingDescs) {
				desc.bindingInfo = binding_infos[n++];
				desc.hashValue   = desc.bindingInfo.hash();
			}

			break;
		}

		array_desc.bindingDescs.resize(layout_binding.resourceCount);
		for (auto& desc : array_desc.bindingDescs) {
			desc.bindingInfo = binding_infos[n++];
			desc.hashValue   = desc.bindingInfo.hash();
		}
	}
}

static bool check_layout_compatible(const_ref<ResourceLayout> layout, array_view<ResourceBindingInfo> binding_infos)
{
	auto& layout_impl = CoreObject::getImpl(layout);

	size_t binding_count = binding_infos.size();
	size_t binding_id    = 0;
	size_t n             = 0;
	
	for (; binding_id < binding_count - 1; ++binding_id) {
		const auto& layout_binding = layout_impl.bindings[binding_id];

		for (size_t i = 0; i < layout_binding.resourceCount; ++i) {
			if (binding_infos.size() <= n)
				return false;
			
			const auto& binding_info = binding_infos[n++];

			if (binding_info.dstBinding != binding_id)
				return false;
			if (binding_info.resourceType != layout_binding.resourceType)
				return false;
		}
	}

	const auto&    last_layout_binding = layout_impl.bindings[binding_id];
	const uint32_t last_binding_count  = binding_infos.size() - n;

	for (; n < binding_infos.size(); ++n) {
		const auto& binding_info = binding_infos[n];

		if (binding_info.dstBinding != binding_id)
			return false;
		if (binding_info.resourceType != last_layout_binding.resourceType)
			return false;
	}

	if (last_layout_binding.resourceCount != last_binding_count)
		if (!last_layout_binding.flags.has(ResourceLayoutBindingFlagBits::VariableBindingCount))
			return false;

	return true;
}

obj<ResourceBindingPool> ResourceBindingPool::create(obj<Device> device)
{
	auto  obj       = createNewCoreObject<ResourceBindingPool>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	vk::DescriptorPoolSize pool_sizes[] = {
		{ vk::DescriptorType::eSampler,                1000 },
		{ vk::DescriptorType::eCombinedImageSampler,   1000 },
		{ vk::DescriptorType::eSampledImage,           1000 },
		{ vk::DescriptorType::eStorageImage,           1000 },
		{ vk::DescriptorType::eUniformTexelBuffer,     1000 },
		{ vk::DescriptorType::eStorageTexelBuffer,     1000 },
		{ vk::DescriptorType::eUniformBuffer,          1000 },
		{ vk::DescriptorType::eStorageBuffer,          1000 },
		{ vk::DescriptorType::eUniformBufferDynamic,   1000 },
		{ vk::DescriptorType::eStorageBufferDynamic,   1000 },
		{ vk::DescriptorType::eInputAttachment,        1000 }
	};

	vk::DescriptorPoolCreateInfo pool_info;
	pool_info.maxSets       = 10000;
	pool_info.poolSizeCount = static_cast<uint32_t>(VERA_LENGTHOF(pool_sizes));
	pool_info.pPoolSizes    = pool_sizes;
	
	impl.device         = std::move(device);
	impl.descriptorPool = vk_device.createDescriptorPool(pool_info);

	return obj;
}

ResourceBindingPool::~ResourceBindingPool()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	for (auto& binding : impl.bindingMap) {
		auto& binding_impl = getImpl(binding.second);

		binding_impl.resourceBindingPool = nullptr;
		binding_impl.device              = nullptr;
		binding_impl.resourceLayout      = nullptr;
		binding_impl.descriptorSet       = nullptr;
		binding_impl.hashValue           = 0;
		binding_impl.bindingStates.clear();
	}

	vk_device.destroy(impl.descriptorPool);

	destroyObjectImpl(this);
}

obj<Device> ResourceBindingPool::getDevice() VERA_NOEXCEPT
{
	auto& impl = getImpl(this);
	return impl.device;
}

obj<ResourceBinding> ResourceBindingPool::allocateBinding(
	const_ref<ResourceLayout> layout
) {
	VERA_ASSERT(layout, "empty resource layout");

	auto  obj          = createNewCoreObject<ResourceBinding>();
	auto& impl         = getImpl(this);
	auto& layout_impl  = getImpl(layout);
	auto& binding_impl = getImpl(obj);
	auto  vk_device    = get_vk_device(impl.device);

	vk::DescriptorSetAllocateInfo alloc_info;
	alloc_info.descriptorPool     = impl.descriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts        = &layout_impl.descriptorSetLayout;

	if (vk_device.allocateDescriptorSets(&alloc_info, &binding_impl.descriptorSet) != vk::Result::eSuccess)
		throw Exception("failed to allocate descriptor set");

	binding_impl.device              = impl.device;
	binding_impl.resourceBindingPool = this;
	binding_impl.resourceLayout      = layout;
	binding_impl.hashValue           = 0;
	binding_impl.bindingStates       = {};
	binding_impl.arrayElementCount   = layout_impl.bindings.back().resourceCount;
	binding_impl.isCached            = false;

	fill_binding_states(binding_impl, layout_impl, binding_impl.arrayElementCount);

	// resource binding with empty infos, make unique hash from it's address
	hash_combine(binding_impl.hashValue, obj.get());

	// TODO: check hash collison
	impl.bindingMap[binding_impl.hashValue] = obj;

	return obj;
}

obj<ResourceBinding> ResourceBindingPool::allocateBinding(
	const_ref<ResourceLayout> layout,
	uint32_t                  variable_binding_count
) {
	VERA_ASSERT(layout, "empty resource layout");

	auto  obj          = createNewCoreObject<ResourceBinding>();
	auto& impl         = getImpl(this);
	auto& layout_impl  = getImpl(layout);
	auto& binding_impl = getImpl(obj);
	auto  vk_device    = get_vk_device(impl.device);

	// TODO: consider using assert
	if (!layout_impl.bindings.back().flags.has(ResourceLayoutBindingFlagBits::VariableBindingCount))
		throw Exception("last binding of layout is not variable count");

	vk::DescriptorSetVariableDescriptorCountAllocateInfo var_count_info;
	var_count_info.descriptorSetCount = 1;
	var_count_info.pDescriptorCounts  = &variable_binding_count;

	vk::DescriptorSetAllocateInfo alloc_info;
	alloc_info.descriptorPool     = impl.descriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts        = &layout_impl.descriptorSetLayout;
	alloc_info.pNext              = &var_count_info;

	if (vk_device.allocateDescriptorSets(&alloc_info, &binding_impl.descriptorSet) != vk::Result::eSuccess)
		throw Exception("failed to allocate descriptor set");

	binding_impl.device              = impl.device;
	binding_impl.resourceBindingPool = this;
	binding_impl.resourceLayout      = layout;
	binding_impl.hashValue           = 0;
	binding_impl.bindingStates       = {};
	binding_impl.arrayElementCount   = variable_binding_count;
	binding_impl.isCached            = false;

	fill_binding_states(binding_impl, layout_impl, variable_binding_count);

	// resource binding with empty infos, make unique hash from it's address
	hash_combine(binding_impl.hashValue, obj.get());

	// TODO: check hash collison
	impl.bindingMap[binding_impl.hashValue] = obj;

	return obj;
}

obj<ResourceBinding> ResourceBindingPool::requestBinding(
	const_ref<ResourceLayout>       layout,
	array_view<ResourceBindingInfo> binding_infos
) {
	auto& impl = getImpl(this);

	hash_t hash_value = hash_request(layout, binding_infos);

	if (auto it = impl.bindingMap.find(hash_value); it != impl.bindingMap.cend())
		return unsafe_obj_cast<ResourceBinding>(it->second);

	std::vector<ResourceBindingInfo> ordered_infos(VERA_SPAN(binding_infos));
	std::sort(VERA_SPAN(ordered_infos));

	VERA_ASSERT_MSG(check_layout_compatible(layout, ordered_infos), "layout and binding info mismatch");

	auto  obj          = createNewCoreObject<ResourceBinding>();
	auto& binding_impl = getImpl(obj);

	binding_impl.device              = impl.device;
	binding_impl.resourceBindingPool = this;
	binding_impl.resourceLayout      = layout;
	binding_impl.hashValue           = hash_value;
	binding_impl.bindingStates       = {};
	binding_impl.arrayElementCount   = 0;
	binding_impl.isCached            = true;

	fill_binding_states_from_infos(binding_impl, getImpl(layout), binding_infos);

	obj->update();

	impl.bindingMap[binding_impl.hashValue] = obj;

	return obj;
}

void ResourceBindingPool::reset()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	// invalidate all existing bindings
	for (auto& binding : impl.bindingMap) {
		auto& binding_impl = getImpl(binding.second);

		binding_impl.descriptorSet  = nullptr;
	}

	vk_device.resetDescriptorPool(impl.descriptorPool);
}

VERA_NAMESPACE_END
