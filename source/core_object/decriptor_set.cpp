#include "../../include/vera/core/descriptor_set.h"
#include "../impl/descriptor_set_impl.h"
#include "../impl/descriptor_pool_impl.h"
#include "../impl/descriptor_set_layout_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/core/descriptor_pool.h"

VERA_NAMESPACE_BEGIN

static size_t count_total_bindings(DescriptorSetImpl& impl)
{
	size_t count = 0;

	for (const auto& array_desc : impl.bindingStates)
		for (const auto& binding_desc : array_desc.bindingDescs)
			++count;

	return count;
}

static bool check_empty(DescriptorSetImpl& impl)
{
	for (const auto& array_desc : impl.bindingStates)
		for (const auto& binding_desc : array_desc.bindingDescs)
			if (!binding_desc.bindingInfo.empty())
				return false;
	return true;
}

static size_t skip_empty(const DescriptorArrayBindingDesc& array_desc, size_t i)
{
	const size_t array_size = array_desc.bindingDescs.size();

	for (; i < array_size; ++i)
		if (!array_desc.bindingDescs[i].bindingInfo.empty())
			return i;

	return array_size;
}

static void rewrite_binding_info_cached(DescriptorSetImpl& impl, const DescriptorBindingInfo& info)
{
	auto& array_desc   = impl.bindingStates[info.dstBinding];
	auto& binding_desc = array_desc.bindingDescs[info.dstArrayElement];

	hash_unordered(impl.hashValue, binding_desc.hashValue);

	binding_desc.bindingInfo = info;
	binding_desc.hashValue   = info.hash();

	hash_unordered(impl.hashValue, binding_desc.hashValue);
}

static void rewrite_binding_info(DescriptorSetImpl& impl, const DescriptorBindingInfo& info)
{
	auto& array_desc   = impl.bindingStates[info.dstBinding];
	auto& binding_desc = array_desc.bindingDescs[info.dstArrayElement];

	binding_desc.bindingInfo = info;
}

static void reallocate_descriptor_binding(DescriptorSetImpl& impl)
{
	VERA_ASSERT_MSG(!impl.descriptorPool, "descriptor pool destroyed");
	VERA_ASSERT_MSG(!impl.descriptorSet, "descriptor set already allocated");

	auto& pool_impl   = CoreObject::getImpl(impl.descriptorPool);
	auto& layout_impl = CoreObject::getImpl(impl.descriptorSetLayout);
	auto  vk_device   = get_vk_device(impl.device);

	vk::DescriptorSetAllocateInfo alloc_info;
	alloc_info.descriptorPool     = pool_impl.descriptorPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts        = &get_vk_descriptor_set_layout(impl.descriptorSetLayout);

	vk::DescriptorSetVariableDescriptorCountAllocateInfo var_count_info;

	if (layout_impl.bindings.back().flags.has(DescriptorSetLayoutBindingFlagBits::VariableBindingCount)) {
		var_count_info.descriptorSetCount = 1;
		var_count_info.pDescriptorCounts  = &impl.arrayElementCount;
		alloc_info.pNext                  = &var_count_info;
	}

	if (vk_device.allocateDescriptorSets(&alloc_info, &impl.descriptorSet) != vk::Result::eSuccess)
		throw Exception("failed to allocate descriptor set");
}

static void update_descriptor_set_single_info(DescriptorSetImpl& impl, const DescriptorBindingInfo& info)
{
	auto vk_device = get_vk_device(impl.device);

	vk::WriteDescriptorSet write_desc;
	write_desc.dstSet          = impl.descriptorSet;
	write_desc.dstBinding      = info.dstBinding;
	write_desc.dstArrayElement = info.dstArrayElement;
	write_desc.descriptorCount = 1;

	vk::DescriptorBufferInfo buffer_info;
	vk::DescriptorImageInfo  image_info;

	switch (info.descriptorType) {
	case DescriptorType::Sampler:
		image_info.sampler        = get_vk_sampler(info.sampler.sampler);
		write_desc.descriptorType = vk::DescriptorType::eSampler;
		write_desc.pImageInfo     = &image_info;
		break;
	case DescriptorType::CombinedImageSampler:
		image_info.sampler        = get_vk_sampler(info.combinedImageSampler.sampler);
		image_info.imageView      = get_vk_image_view(info.combinedImageSampler.textureView);
		image_info.imageLayout    = to_vk_image_layout(info.combinedImageSampler.textureLayout);
		write_desc.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		write_desc.pImageInfo     = &image_info;
		break;
	case DescriptorType::SampledImage:
	case DescriptorType::StorageImage:
		image_info.imageView      = get_vk_image_view(info.sampledImage.textureView);
		image_info.imageLayout    = to_vk_image_layout(info.sampledImage.textureLayout);
		write_desc.descriptorType = to_vk_descriptor_type(info.descriptorType);
		write_desc.pImageInfo     = &image_info;
		break;
	case DescriptorType::UniformTexelBuffer:
	case DescriptorType::StorageTexelBuffer:
		write_desc.descriptorType   = to_vk_descriptor_type(info.descriptorType);
		write_desc.pTexelBufferView = &get_vk_buffer_view(info.uniformTexelBuffer.bufferView);
		break;
	case DescriptorType::UniformBuffer:
	case DescriptorType::StorageBuffer:
	case DescriptorType::UniformBufferDynamic:
	case DescriptorType::StorageBufferDynamic:
		buffer_info.buffer        = get_vk_buffer(info.uniformBuffer.buffer);
		buffer_info.offset        = info.uniformBuffer.offset;
		buffer_info.range         = info.uniformBuffer.range;
		write_desc.descriptorType = to_vk_descriptor_type(info.descriptorType);
		write_desc.pBufferInfo    = &buffer_info;
		break;
	case DescriptorType::InputAttachment:
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
	const DescriptorArrayBindingDesc&     array_desc
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
		case DescriptorType::Sampler: {
			auto& image_info = image_infos.emplace_back();
			image_info.sampler        = get_vk_sampler(binding_info.sampler.sampler);
			write_desc.descriptorType = vk::DescriptorType::eSampler;
		} break;
		case DescriptorType::CombinedImageSampler: {
			auto& image_info = image_infos.emplace_back();
			image_info.sampler     = get_vk_sampler(binding_info.combinedImageSampler.sampler);
			image_info.imageView   = get_vk_image_view(binding_info.combinedImageSampler.textureView);
			image_info.imageLayout = to_vk_image_layout(binding_info.combinedImageSampler.textureLayout);
		} break;
		case DescriptorType::SampledImage:
		case DescriptorType::StorageImage:
		case DescriptorType::InputAttachment: {
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
	vk::Device                        vk_device,
	vk::DescriptorSet                 vk_descriptor_set,
	std::vector<vk::BufferView>&      buffer_views,
	const DescriptorArrayBindingDesc& array_desc
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
	const DescriptorArrayBindingDesc&      array_desc
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

const vk::DescriptorSet& get_vk_descriptor_set(const_ref<DescriptorSet> descriptor_set)
{
	return CoreObject::getImpl(descriptor_set).descriptorSet;
}

vk::DescriptorSet& get_vk_descriptor_set(ref<DescriptorSet> descriptor_set)
{
	return CoreObject::getImpl(descriptor_set).descriptorSet;
}

DescriptorBindingInfo::DescriptorBindingInfo() VERA_NOEXCEPT :
	descriptorType(DescriptorType::Unknown),
	dstBinding(0),
	dstArrayElement(0) {}

DescriptorBindingInfo::DescriptorBindingInfo(const DescriptorBindingInfo& rhs) VERA_NOEXCEPT
{
	memcpy(this, &rhs, sizeof(DescriptorBindingInfo));
}

DescriptorBindingInfo::~DescriptorBindingInfo()
{
}

DescriptorBindingInfo& DescriptorBindingInfo::operator=(const DescriptorBindingInfo& rhs) VERA_NOEXCEPT
{
	if (this != &rhs)
		memcpy(this, &rhs, sizeof(DescriptorBindingInfo));
	
	return *this;
}

bool DescriptorBindingInfo::operator<(const DescriptorBindingInfo& rhs) const VERA_NOEXCEPT
{
	if (dstBinding != rhs.dstBinding)
		return dstBinding < rhs.dstBinding;

	return dstArrayElement < rhs.dstArrayElement;
}

bool DescriptorBindingInfo::empty() const VERA_NOEXCEPT
{
	return descriptorType == DescriptorType::Unknown;
}

hash_t DescriptorBindingInfo::hash() const VERA_NOEXCEPT
{
	hash_t seed = 0;
	
	hash_combine(seed, static_cast<uint32_t>(descriptorType));
	hash_combine(seed, dstBinding);
	hash_combine(seed, dstArrayElement);

	switch (descriptorType) {
	case DescriptorType::Sampler:
		hash_combine(seed, sampler.sampler.get());
		break;
	case DescriptorType::CombinedImageSampler:
		hash_combine(seed, combinedImageSampler.sampler.get());
		hash_combine(seed, combinedImageSampler.textureView.get());
		hash_combine(seed, static_cast<uint32_t>(combinedImageSampler.textureLayout));
		break;
	case DescriptorType::SampledImage:
	case DescriptorType::StorageImage:
		hash_combine(seed, sampledImage.textureView.get());
		hash_combine(seed, static_cast<uint32_t>(sampledImage.textureLayout));
		break;
	case DescriptorType::UniformTexelBuffer:
	case DescriptorType::StorageTexelBuffer:
		hash_combine(seed, uniformTexelBuffer.bufferView.get());
		break;
	case DescriptorType::UniformBuffer:
	case DescriptorType::StorageBuffer:
	case DescriptorType::UniformBufferDynamic:
	case DescriptorType::StorageBufferDynamic:
		hash_combine(seed, uniformBuffer.buffer.get());
		hash_combine(seed, uniformBuffer.offset);
		hash_combine(seed, uniformBuffer.range);
		break;
	case DescriptorType::InputAttachment:
		hash_combine(seed, sampledImage.textureView.get());
		hash_combine(seed, static_cast<uint32_t>(sampledImage.textureLayout));
		break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource type");
	}

	return seed;
}

DescriptorSet::~DescriptorSet()
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.descriptorPool);

	pool_impl.descriptorSetMap.erase(impl.hashValue);

	destroyObjectImpl(this);
}

ref<DescriptorPool> DescriptorSet::getDescriptorPool() VERA_NOEXCEPT
{
	return getImpl(this).descriptorPool;
}

const_ref<DescriptorSetLayout> DescriptorSet::getDescriptorSetLayout() VERA_NOEXCEPT
{
	return getImpl(this).descriptorSetLayout;
}

const DescriptorBindingInfo& DescriptorSet::getDescriptorBindingInfo(uint32_t binding, uint32_t array_element) VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	VERA_ASSERT_MSG(binding < impl.bindingStates.size(), "binding index out of range");
	
	const auto& array_desc = impl.bindingStates[binding];
	
	VERA_ASSERT_MSG(array_element < array_desc.bindingDescs.size(), "array element index out of range");
	
	return array_desc.bindingDescs[array_element].bindingInfo;
}

void DescriptorSet::setDescriptorBindingInfo(const DescriptorBindingInfo& info)
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.descriptorPool);

	if (!impl.descriptorSet)
		reallocate_descriptor_binding(impl);

	if (impl.isCached) {
		pool_impl.descriptorSetMap.erase(impl.hashValue);

		rewrite_binding_info_cached(impl, info);

		pool_impl.descriptorSetMap.insert(std::make_pair(impl.hashValue, ref<DescriptorSet>(this)));
	} else {
		rewrite_binding_info(impl, info);
	}

	update_descriptor_set_single_info(impl, info);
}

void DescriptorSet::setDescriptorBindingInfo(array_view<DescriptorBindingInfo> infos)
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.descriptorPool);

	if (!impl.descriptorSet)
		reallocate_descriptor_binding(impl);

	if (impl.isCached) {
		pool_impl.descriptorSetMap.erase(impl.hashValue);

		for (const auto& info : infos)
			rewrite_binding_info_cached(impl, info);

		pool_impl.descriptorSetMap.insert(std::make_pair(impl.hashValue, ref<DescriptorSet>(this)));
	} else {
		for (const auto& info : infos)
			rewrite_binding_info(impl, info);
	}

	// TODO: implement batch update
	for (const auto& info : infos)
		update_descriptor_set_single_info(impl, info);
}

void DescriptorSet::update()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	if (!impl.descriptorSet)
		reallocate_descriptor_binding(impl);

	std::vector<vk::DescriptorBufferInfo> buffer_infos;
	std::vector<vk::DescriptorImageInfo>  image_infos;
	std::vector<vk::BufferView>           buffer_views;

	for (const auto& array_desc : impl.bindingStates) {
		switch (array_desc.resourceType) {
		case DescriptorType::Sampler:
		case DescriptorType::CombinedImageSampler:
		case DescriptorType::SampledImage:
		case DescriptorType::StorageImage:
		case DescriptorType::InputAttachment:
			update_descriptor_image_info(vk_device, impl.descriptorSet, image_infos, array_desc);
			break;
		case DescriptorType::UniformTexelBuffer:
		case DescriptorType::StorageTexelBuffer:
			update_descriptor_texel_buffer_info(vk_device, impl.descriptorSet, buffer_views, array_desc);
			break;
		case DescriptorType::UniformBuffer:
		case DescriptorType::StorageBuffer:
		case DescriptorType::UniformBufferDynamic:
		case DescriptorType::StorageBufferDynamic:
			update_descriptor_buffer_info(vk_device, impl.descriptorSet, buffer_infos, array_desc);
			break;
		}
	}
}

void DescriptorSet::makeCached()
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.descriptorPool);

	if (check_empty(impl))
		throw Exception("to make resource binding cached all of resource are should updated");

	pool_impl.descriptorSetMap.erase(impl.hashValue);

	impl.hashValue = 0;

	hash_combine(impl.hashValue, impl.descriptorSetLayout->hash());

	hash_combine(impl.hashValue, count_total_bindings(impl));
	for (auto& array_desc : impl.bindingStates) {
		for (auto& binding_desc : array_desc.bindingDescs) {
			binding_desc.hashValue = binding_desc.bindingInfo.hash();
			hash_unordered(impl.hashValue, binding_desc.hashValue);
		}
	}
	
	pool_impl.descriptorSetMap.insert(std::make_pair(impl.hashValue, ref<DescriptorSet>(this)));
	
	impl.isCached = true;
}

bool DescriptorSet::isCached() const VERA_NOEXCEPT
{
	return getImpl(this).isCached;
}

bool DescriptorSet::isValid() const VERA_NOEXCEPT
{
	return static_cast<bool>(getImpl(this).descriptorSet);
}

bool DescriptorSet::isDestroyed() const VERA_NOEXCEPT
{
	return static_cast<bool>(getImpl(this).descriptorPool);
}

hash_t DescriptorSet::hashValue() const VERA_NOEXCEPT
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END