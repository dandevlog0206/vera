#include "../../include/vera/core/resource_binding.h"
#include "../impl/resource_binding_impl.h"
#include "../impl/resource_layout_impl.h"
#include "../impl/resource_binding_pool_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/resource_layout.h"
#include "../../include/vera/core/resource_binding_pool.h"

VERA_NAMESPACE_BEGIN

static size_t count_total_bindings(ResourceBindingImpl& impl)
{
	size_t count = 0;

	for (const auto& array_desc : impl.bindingStates)
		for (const auto& binding_desc : array_desc.bindingDescs)
			++count;

	return count;
}

static bool check_empty(ResourceBindingImpl& impl)
{
	for (const auto& array_desc : impl.bindingStates)
		for (const auto& binding_desc : array_desc.bindingDescs)
			if (!binding_desc.bindingInfo.empty())
				return false;
	return true;
}

static size_t skip_empty(const ResourceArrayBindingDesc& array_desc, size_t i)
{
	const size_t array_size = array_desc.bindingDescs.size();

	for (; i < array_size; ++i)
		if (!array_desc.bindingDescs[i].bindingInfo.empty())
			return i;

	return array_size;
}

static void rewrite_binding_info_cached(ResourceBindingImpl& impl, const ResourceBindingInfo& info)
{
	auto& array_desc   = impl.bindingStates[info.dstBinding];
	auto& binding_desc = array_desc.bindingDescs[info.dstArrayElement];

	hash_unordered(impl.hashValue, binding_desc.hashValue);

	binding_desc.bindingInfo = info;
	binding_desc.hashValue   = info.hash();

	hash_unordered(impl.hashValue, binding_desc.hashValue);
}

static void rewrite_binding_info(ResourceBindingImpl& impl, const ResourceBindingInfo& info)
{
	auto& array_desc   = impl.bindingStates[info.dstBinding];
	auto& binding_desc = array_desc.bindingDescs[info.dstArrayElement];

	binding_desc.bindingInfo = info;
}

static void reallocate_resource_binding(ResourceBindingImpl& impl)
{
	VERA_ASSERT_MSG(!impl.resourceBindingPool, "binding pool destroyed");
	VERA_ASSERT_MSG(!impl.descriptorSet, "descriptor set already allocated");

	auto& pool_impl   = CoreObject::getImpl(impl.resourceBindingPool);
	auto& layout_impl = CoreObject::getImpl(impl.resourceLayout);
	auto  vk_device   = get_vk_device(impl.device);

	vk::DescriptorSetAllocateInfo alloc_info;
	alloc_info.descriptorPool     = pool_impl.descriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts        = &get_vk_descriptor_set_layout(impl.resourceLayout);

	vk::DescriptorSetVariableDescriptorCountAllocateInfo var_count_info;
	
	if (layout_impl.bindings.back().flags.has(ResourceLayoutBindingFlagBits::VariableBindingCount)) {
		var_count_info.descriptorSetCount = 1;
		var_count_info.pDescriptorCounts  = &impl.arrayElementCount;
		alloc_info.pNext                  = &var_count_info;
	}

	if (vk_device.allocateDescriptorSets(&alloc_info, &impl.descriptorSet) != vk::Result::eSuccess)
		throw Exception("failed to allocate descriptor set");
}

static void update_descriptor_set_single_info(ResourceBindingImpl& impl, const ResourceBindingInfo& info)
{
	auto vk_device = get_vk_device(impl.device);

	vk::WriteDescriptorSet write_desc;
	write_desc.dstSet          = impl.descriptorSet;
	write_desc.dstBinding      = info.dstBinding;
	write_desc.dstArrayElement = info.dstArrayElement;
	write_desc.descriptorCount = 1;

	vk::DescriptorBufferInfo buffer_info;
	vk::DescriptorImageInfo  image_info;

	switch (info.resourceType) {
	case ResourceType::Sampler:
		image_info.sampler        = get_vk_sampler(info.sampler.sampler);
		write_desc.descriptorType = vk::DescriptorType::eSampler;
		write_desc.pImageInfo     = &image_info;
		break;
	case ResourceType::CombinedImageSampler:
		image_info.sampler        = get_vk_sampler(info.combinedImageSampler.sampler);
		image_info.imageView      = get_vk_image_view(info.combinedImageSampler.textureView);
		image_info.imageLayout    = to_vk_image_layout(info.combinedImageSampler.textureLayout);
		write_desc.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		write_desc.pImageInfo     = &image_info;
		break;
	case ResourceType::SampledImage:
	case ResourceType::StorageImage:
		image_info.imageView      = get_vk_image_view(info.sampledImage.textureView);
		image_info.imageLayout    = to_vk_image_layout(info.sampledImage.textureLayout);
		write_desc.descriptorType = to_vk_descriptor_type(info.resourceType);
		write_desc.pImageInfo     = &image_info;
		break;
	case ResourceType::UniformTexelBuffer:
	case ResourceType::StorageTexelBuffer:
		write_desc.descriptorType   = to_vk_descriptor_type(info.resourceType);
		write_desc.pTexelBufferView = &get_vk_buffer_view(info.uniformTexelBuffer.bufferView);
		break;
	case ResourceType::UniformBuffer:
	case ResourceType::StorageBuffer:
	case ResourceType::UniformBufferDynamic:
	case ResourceType::StorageBufferDynamic:
		buffer_info.buffer        = get_vk_buffer(info.uniformBuffer.buffer);
		buffer_info.offset        = info.uniformBuffer.offset;
		buffer_info.range         = info.uniformBuffer.range;
		write_desc.descriptorType = to_vk_descriptor_type(info.resourceType);
		write_desc.pBufferInfo    = &buffer_info;
		break;
	case ResourceType::InputAttachment:
		image_info.imageView      = get_vk_image_view(info.sampledImage.textureView);
		image_info.imageLayout    = to_vk_image_layout(info.sampledImage.textureLayout);
		write_desc.descriptorType = vk::DescriptorType::eInputAttachment;
		write_desc.pImageInfo     = &image_info;
		break;
	}

	vk_device.updateDescriptorSets(1, &write_desc, 0, nullptr);
}

static void update_descriptor_image_info(
	vk::Device                            vk_device,
	vk::DescriptorSet                     vk_descriptor_set,
	std::vector<vk::DescriptorImageInfo>& image_infos,
	const ResourceArrayBindingDesc&       array_desc
) {
	vk::WriteDescriptorSet write_desc;
	write_desc.dstSet         = vk_descriptor_set;
	write_desc.dstBinding     = array_desc.binding;
	write_desc.descriptorType = to_vk_descriptor_type(array_desc.resourceType);

	for (size_t i = skip_empty(array_desc, 0); i < array_desc.bindingDescs.size();) {
		const auto& binding_info = array_desc.bindingDescs[i].bindingInfo;

		if (binding_info.empty()) {
			write_desc.dstArrayElement = i;
			write_desc.descriptorCount = static_cast<uint32_t>(image_infos.size());
			write_desc.pImageInfo      = image_infos.data();

			vk_device.updateDescriptorSets(1, &write_desc, 0, nullptr);
			image_infos.clear();

			i = skip_empty(array_desc, i);
			continue;
		}
		
		switch (array_desc.resourceType) {
		case ResourceType::Sampler: {
			auto& image_info = image_infos.emplace_back();
			image_info.sampler        = get_vk_sampler(binding_info.sampler.sampler);
			write_desc.descriptorType = vk::DescriptorType::eSampler;
		} break;
		case ResourceType::CombinedImageSampler: {
			auto& image_info = image_infos.emplace_back();
			image_info.sampler     = get_vk_sampler(binding_info.combinedImageSampler.sampler);
			image_info.imageView   = get_vk_image_view(binding_info.combinedImageSampler.textureView);
			image_info.imageLayout = to_vk_image_layout(binding_info.combinedImageSampler.textureLayout);
		} break;
		case ResourceType::SampledImage:
		case ResourceType::StorageImage:
		case ResourceType::InputAttachment: {
			auto& image_info = image_infos.emplace_back();
			image_info.imageView   = get_vk_image_view(binding_info.sampledImage.textureView);
			image_info.imageLayout = to_vk_image_layout(binding_info.sampledImage.textureLayout);
		} break;
		default:
			VERA_ASSERT_MSG(false, "invalid resource type for image info");
		}

		++i;
	}
}

static void update_descriptor_texel_buffer_info(
	vk::Device                      vk_device,
	vk::DescriptorSet               vk_descriptor_set,
	std::vector<vk::BufferView>&    buffer_views,
	const ResourceArrayBindingDesc& array_desc
) {
	vk::WriteDescriptorSet write_desc;
	write_desc.dstSet         = vk_descriptor_set;
	write_desc.dstBinding     = array_desc.binding;
	write_desc.descriptorType = to_vk_descriptor_type(array_desc.resourceType);

	for (size_t i = skip_empty(array_desc, 0); i < array_desc.bindingDescs.size();) {
		const auto& binding_info = array_desc.bindingDescs[i].bindingInfo;

		if (binding_info.empty()) {
			write_desc.dstArrayElement  = i;
			write_desc.descriptorCount  = static_cast<uint32_t>(buffer_views.size());
			write_desc.pTexelBufferView = buffer_views.data();

			vk_device.updateDescriptorSets(1, &write_desc, 0, nullptr);
			buffer_views.clear();

			i = skip_empty(array_desc, i);
			continue;
		}
		
		buffer_views.push_back(get_vk_buffer_view(binding_info.uniformTexelBuffer.bufferView));

		++i;
	}
}

static void update_descriptor_buffer_info(
	vk::Device                             vk_device,
	vk::DescriptorSet                      vk_descriptor_set,
	std::vector<vk::DescriptorBufferInfo>& buffer_infos,
	const ResourceArrayBindingDesc&        array_desc
) {
	vk::WriteDescriptorSet write_desc;
	write_desc.dstSet         = vk_descriptor_set;
	write_desc.dstBinding     = array_desc.binding;
	write_desc.descriptorType = to_vk_descriptor_type(array_desc.resourceType);

	for (size_t i = skip_empty(array_desc, 0); i < array_desc.bindingDescs.size();) {
		const auto& binding_info = array_desc.bindingDescs[i].bindingInfo;

		if (binding_info.empty()) {
			write_desc.dstArrayElement = i;
			write_desc.descriptorCount = static_cast<uint32_t>(buffer_infos.size());
			write_desc.pBufferInfo     = buffer_infos.data();

			vk_device.updateDescriptorSets(1, &write_desc, 0, nullptr);
			buffer_infos.clear();

			i = skip_empty(array_desc, i);
			continue;
		}

		auto& buffer_info = buffer_infos.emplace_back();
		buffer_info.buffer = get_vk_buffer(binding_info.uniformBuffer.buffer);
		buffer_info.offset = binding_info.uniformBuffer.offset;
		buffer_info.range  = binding_info.uniformBuffer.range;

		++i;
	}
}

ResourceBindingInfo::ResourceBindingInfo() VERA_NOEXCEPT :
	resourceType(ResourceType::Unknown),
	dstBinding(0),
	dstArrayElement(0) {}

ResourceBindingInfo::ResourceBindingInfo(const ResourceBindingInfo& rhs) VERA_NOEXCEPT
{
	memcpy(this, &rhs, sizeof(ResourceBindingInfo));
}

ResourceBindingInfo::~ResourceBindingInfo()
{
}

ResourceBindingInfo& ResourceBindingInfo::operator=(const ResourceBindingInfo& rhs) VERA_NOEXCEPT
{
	if (this != &rhs)
		memcpy(this, &rhs, sizeof(ResourceBindingInfo));
	
	return *this;
}

bool ResourceBindingInfo::operator<(const ResourceBindingInfo& rhs) const VERA_NOEXCEPT
{
	if (dstBinding != rhs.dstBinding)
		return dstBinding < rhs.dstBinding;

	return dstArrayElement < rhs.dstArrayElement;
}

bool ResourceBindingInfo::empty() const VERA_NOEXCEPT
{
	return resourceType == ResourceType::Unknown;
}

hash_t ResourceBindingInfo::hash() const VERA_NOEXCEPT
{
	hash_t seed = 0;
	
	hash_combine(seed, static_cast<uint32_t>(resourceType));
	hash_combine(seed, dstBinding);
	hash_combine(seed, dstArrayElement);

	switch (resourceType) {
	case ResourceType::Sampler:
		hash_combine(seed, sampler.sampler.get());
		break;
	case ResourceType::CombinedImageSampler:
		hash_combine(seed, combinedImageSampler.sampler.get());
		hash_combine(seed, combinedImageSampler.textureView.get());
		hash_combine(seed, static_cast<uint32_t>(combinedImageSampler.textureLayout));
		break;
	case ResourceType::SampledImage:
	case ResourceType::StorageImage:
		hash_combine(seed, sampledImage.textureView.get());
		hash_combine(seed, static_cast<uint32_t>(sampledImage.textureLayout));
		break;
	case ResourceType::UniformTexelBuffer:
	case ResourceType::StorageTexelBuffer:
		hash_combine(seed, uniformTexelBuffer.bufferView.get());
		break;
	case ResourceType::UniformBuffer:
	case ResourceType::StorageBuffer:
	case ResourceType::UniformBufferDynamic:
	case ResourceType::StorageBufferDynamic:
		hash_combine(seed, uniformBuffer.buffer.get());
		hash_combine(seed, uniformBuffer.offset);
		hash_combine(seed, uniformBuffer.range);
		break;
	case ResourceType::InputAttachment:
		hash_combine(seed, sampledImage.textureView.get());
		hash_combine(seed, static_cast<uint32_t>(sampledImage.textureLayout));
		break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource type");
	}

	return seed;
}

ResourceBinding::~ResourceBinding()
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.resourceBindingPool);

	pool_impl.bindingMap.erase(impl.hashValue);

	destroyObjectImpl(this);
}

ref<ResourceBindingPool> ResourceBinding::getResourceBindingPool() VERA_NOEXCEPT
{
	return getImpl(this).resourceBindingPool;
}

const_ref<ResourceLayout> ResourceBinding::getResourceLayout() VERA_NOEXCEPT
{
	return getImpl(this).resourceLayout;
}

const ResourceBindingInfo& ResourceBinding::getBindingInfo(uint32_t binding, uint32_t array_element) VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	VERA_ASSERT_MSG(binding < impl.bindingStates.size(), "binding index out of range");
	
	const auto& array_desc = impl.bindingStates[binding];
	
	VERA_ASSERT_MSG(array_element < array_desc.bindingDescs.size(), "array element index out of range");
	
	return array_desc.bindingDescs[array_element].bindingInfo;
}

void ResourceBinding::setBindingInfo(const ResourceBindingInfo& info)
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.resourceBindingPool);

	if (!impl.descriptorSet)
		reallocate_resource_binding(impl);

	if (impl.isCached) {
		pool_impl.bindingMap.erase(impl.hashValue);

		rewrite_binding_info_cached(impl, info);

		pool_impl.bindingMap.insert(std::make_pair(impl.hashValue, ref<ResourceBinding>(this)));
	} else {
		rewrite_binding_info(impl, info);
	}

	update_descriptor_set_single_info(impl, info);
}

void ResourceBinding::setBindingInfo(array_view<ResourceBindingInfo> infos)
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.resourceBindingPool);

	if (!impl.descriptorSet)
		reallocate_resource_binding(impl);

	if (impl.isCached) {
		pool_impl.bindingMap.erase(impl.hashValue);

		for (const auto& info : infos)
			rewrite_binding_info_cached(impl, info);

		pool_impl.bindingMap.insert(std::make_pair(impl.hashValue, ref<ResourceBinding>(this)));
	} else {
		for (const auto& info : infos)
			rewrite_binding_info(impl, info);
	}

	// TODO: implement batch update
	for (const auto& info : infos)
		update_descriptor_set_single_info(impl, info);
}

void ResourceBinding::update()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	if (!impl.descriptorSet)
		reallocate_resource_binding(impl);

	std::vector<vk::DescriptorBufferInfo> buffer_infos;
	std::vector<vk::DescriptorImageInfo>  image_infos;
	std::vector<vk::BufferView>           buffer_views;

	for (const auto& array_desc : impl.bindingStates) {
		switch (array_desc.resourceType) {
		case ResourceType::Sampler:
		case ResourceType::CombinedImageSampler:
		case ResourceType::SampledImage:
		case ResourceType::StorageImage:
		case ResourceType::InputAttachment:
			update_descriptor_image_info(vk_device, impl.descriptorSet, image_infos, array_desc);
			break;
		case ResourceType::UniformTexelBuffer:
		case ResourceType::StorageTexelBuffer:
			update_descriptor_texel_buffer_info(vk_device, impl.descriptorSet, buffer_views, array_desc);
			break;
		case ResourceType::UniformBuffer:
		case ResourceType::StorageBuffer:
		case ResourceType::UniformBufferDynamic:
		case ResourceType::StorageBufferDynamic:
			update_descriptor_buffer_info(vk_device, impl.descriptorSet, buffer_infos, array_desc);
			break;
		}
	}
}

void ResourceBinding::makeCached()
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.resourceBindingPool);

	if (check_empty(impl))
		throw Exception("to make resource binding cached all of resource are should updated");

	pool_impl.bindingMap.erase(impl.hashValue);

	impl.hashValue = 0;

	hash_combine(impl.hashValue, impl.resourceLayout->hash());

	hash_combine(impl.hashValue, count_total_bindings(impl));
	for (auto& array_desc : impl.bindingStates) {
		for (auto& binding_desc : array_desc.bindingDescs) {
			binding_desc.hashValue = binding_desc.bindingInfo.hash();
			hash_unordered(impl.hashValue, binding_desc.hashValue);
		}
	}
	
	pool_impl.bindingMap.insert(std::make_pair(impl.hashValue, ref<ResourceBinding>(this)));
	
	impl.isCached = true;
}

bool ResourceBinding::isCached() const VERA_NOEXCEPT
{
	return getImpl(this).isCached;
}

bool ResourceBinding::isValid() const VERA_NOEXCEPT
{
	return static_cast<bool>(getImpl(this).descriptorSet);
}

bool ResourceBinding::isDestroyed() const VERA_NOEXCEPT
{
	return static_cast<bool>(getImpl(this).resourceBindingPool);
}

hash_t ResourceBinding::hashValue() const VERA_NOEXCEPT
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END