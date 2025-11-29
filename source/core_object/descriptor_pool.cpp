#include "../../include/vera/core/descriptor_pool.h"
#include "../impl/descriptor_pool_impl.h"
#include "../impl/descriptor_set_impl.h"
#include "../impl/descriptor_set_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

static void invalidate_all_descriptor_sets(DescriptorPoolImpl& impl, bool destroyed = false)
{
	if (destroyed) {
		for (auto& [hash, set] : impl.allocatedSets) {
			auto& set_impl = CoreObject::getImpl(set);
			set_impl.device                  = nullptr;
			set_impl.descriptorPool          = nullptr;
			set_impl.descriptorSetLayout     = nullptr;
			set_impl.vkDescriptorSet         = nullptr;
			set_impl.bindingStates.clear();
			set_impl.variableDescriptorCount = 0;
		}
	} else {
		for (auto& [hash, set] : impl.allocatedSets) {
			auto& set_impl = CoreObject::getImpl(set);
			set_impl.vkDescriptorSet = nullptr;
			set_impl.bindingStates.clear();
		}
	}

	impl.allocatedSets.clear();
}

const vk::DescriptorPool& get_vk_descriptor_pool(cref<DescriptorPool> descriptor_pool) VERA_NOEXCEPT
{
	return CoreObject::getImpl(descriptor_pool).vkDescriptorPool;
}

vk::DescriptorPool& get_vk_descriptor_pool(ref<DescriptorPool> descriptor_pool) VERA_NOEXCEPT
{
	return CoreObject::getImpl(descriptor_pool).vkDescriptorPool;
}

obj<DescriptorPool> DescriptorPool::create(obj<Device> device, const DescriptorPoolCreateInfo& info)
{
	static const vk::DescriptorPoolSize default_pool_sizes[] = {
		{ vk::DescriptorType::eSampler,                            1000 },
		{ vk::DescriptorType::eCombinedImageSampler,               1000 },
		{ vk::DescriptorType::eSampledImage,                       1000 },
		{ vk::DescriptorType::eStorageImage,                       1000 },
		{ vk::DescriptorType::eUniformTexelBuffer,                 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer,                 1000 },
		{ vk::DescriptorType::eUniformBuffer,                      1000 },
		{ vk::DescriptorType::eStorageBuffer,                      1000 },
		{ vk::DescriptorType::eUniformBufferDynamic,               1000 },
		{ vk::DescriptorType::eStorageBufferDynamic,               1000 },
		{ vk::DescriptorType::eInputAttachment,                    1000 },

		// TODO: enable descriptor pool sizes for the following descriptor types when supported
		// { vk::DescriptorType::eInlineUniformBlock,                 1000 },
		// { vk::DescriptorType::eAccelerationStructureKHR,           1000 },
		// { vk::DescriptorType::eAccelerationStructureNV,            1000 },
		// { vk::DescriptorType::ePartitionedAccelerationStructureNV, 1000 }
	};

	auto  obj       = createNewCoreObject<DescriptorPool>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	vk::DescriptorPoolCreateInfo pool_info;
	pool_info.flags = to_vk_descriptor_pool_create_flags(info.flags);

	if (info.poolSizes.empty()) {
		pool_info.maxSets       = info.maxSets;
		pool_info.poolSizeCount = static_cast<uint32_t>(VERA_LENGTHOF(default_pool_sizes));
		pool_info.pPoolSizes    = default_pool_sizes;

		if (pool_info.maxSets == 0)
			pool_info.maxSets = static_cast<uint32_t>(pool_info.poolSizeCount * 1000);
	} else {
		pool_info.maxSets       = info.maxSets;
		pool_info.poolSizeCount = static_cast<uint32_t>(info.poolSizes.size());
		pool_info.pPoolSizes    = reinterpret_cast<const vk::DescriptorPoolSize*>(info.poolSizes.data());

		if (pool_info.maxSets == 0)
			for (const auto& size : info.poolSizes)
				pool_info.maxSets += size.size;
	}
	
	impl.device           = std::move(device);
	impl.vkDescriptorPool = vk_device.createDescriptorPool(pool_info);
	impl.flags            = info.flags;
	impl.maxSets          = pool_info.maxSets;

	impl.poolSizes.assign(
		reinterpret_cast<const DescriptorPoolSize*>(pool_info.pPoolSizes),
		reinterpret_cast<const DescriptorPoolSize*>(pool_info.pPoolSizes) + pool_info.poolSizeCount
	);

	return obj;
}

DescriptorPool::~DescriptorPool() VERA_NOEXCEPT
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	invalidate_all_descriptor_sets(impl, true);
	impl.poolMap.clear();

	vk_device.destroy(impl.vkDescriptorPool);

	destroyObjectImpl(this);
}

obj<Device> DescriptorPool::getDevice() VERA_NOEXCEPT
{
	auto& impl = getImpl(this);
	return impl.device;
}

DescriptorPoolCreateFlags DescriptorPool::getFlags() const VERA_NOEXCEPT
{
	return getImpl(this).flags;
}

array_view<DescriptorPoolSize> DescriptorPool::enumeratePoolSizes() const VERA_NOEXCEPT
{
	return getImpl(this).poolSizes;
}

uint32_t DescriptorPool::getMaxSets() const VERA_NOEXCEPT
{
	return getImpl(this).maxSets;
}

obj<DescriptorSet> DescriptorPool::allocate(obj<DescriptorSetLayout> layout) {
	const auto& layout_impl = getImpl(layout);
	const auto& last_binding = layout_impl.bindings.back();

	auto  obj       = createNewCoreObject<DescriptorSet>();
	auto& set_impl  = getImpl(obj);
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk::DescriptorSetAllocateInfo alloc_info;
	alloc_info.descriptorPool     = impl.vkDescriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts        = &layout_impl.vkDescriptorSetLayout;

	if (vk_device.allocateDescriptorSets(&alloc_info, &set_impl.vkDescriptorSet) != vk::Result::eSuccess)
		throw Exception("failed to allocate descriptor set");

	set_impl.device                  = impl.device;
	set_impl.descriptorPool          = this;
	set_impl.descriptorSetLayout     = layout;
	set_impl.bindingStates           = {};
	set_impl.variableDescriptorCount = 0;

	hash_t seed = 0;
	hash_combine(seed, static_cast<VkDescriptorSet>(set_impl.vkDescriptorSet));

	impl.allocatedSets.insert({ seed, obj });

	return obj;
}

obj<DescriptorSet> DescriptorPool::allocate(obj<DescriptorSetLayout> layout, uint32_t variable_descriptor_count)
{
	if (variable_descriptor_count == 1)
		return allocate(layout);

	const auto& layout_impl = getImpl(layout);
	const auto& last_binding = layout_impl.bindings.back();

	if (!last_binding.flags.has(DescriptorSetLayoutBindingFlagBits::VariableDescriptorCount))
		throw Exception("last binding of layout is not variable count");

	auto  obj       = createNewCoreObject<DescriptorSet>();
	auto& impl      = getImpl(this);
	auto& set_impl  = getImpl(obj);
	auto  vk_device = get_vk_device(impl.device);

	vk::DescriptorSetVariableDescriptorCountAllocateInfo count_info;
	count_info.descriptorSetCount = 1;
	count_info.pDescriptorCounts  = &variable_descriptor_count;

	vk::DescriptorSetAllocateInfo alloc_info;
	alloc_info.descriptorPool     = impl.vkDescriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts        = &layout_impl.vkDescriptorSetLayout;
	alloc_info.pNext              = &count_info;

	if (vk_device.allocateDescriptorSets(&alloc_info, &set_impl.vkDescriptorSet) != vk::Result::eSuccess)
		throw Exception("failed to allocate descriptor set");

	set_impl.device                  = impl.device;
	set_impl.descriptorPool          = this;
	set_impl.descriptorSetLayout     = layout;
	set_impl.bindingStates           = {};
	set_impl.variableDescriptorCount = variable_descriptor_count;

	hash_t seed = 0;
	hash_combine(seed, static_cast<VkDescriptorSet>(set_impl.vkDescriptorSet));

	impl.allocatedSets.insert({ seed, obj });

	return obj;
}

void DescriptorPool::reset()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	invalidate_all_descriptor_sets(impl);
	impl.poolMap.clear();

	vk_device.resetDescriptorPool(impl.vkDescriptorPool);
}

VERA_NAMESPACE_END
