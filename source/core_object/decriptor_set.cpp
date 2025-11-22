#include "../../include/vera/core/descriptor_set.h"
#include "../impl/descriptor_set_impl.h"
#include "../impl/descriptor_pool_impl.h"
#include "../impl/descriptor_set_layout_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/core/descriptor_pool.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"

VERA_NAMESPACE_BEGIN

template <class ObjType>
static VERA_CONSTEXPR uint64_t descriptor_idx_v = 0;
template <> static VERA_CONSTEXPR uint64_t descriptor_idx_v<Sampler>     = 1;
template <> static VERA_CONSTEXPR uint64_t descriptor_idx_v<TextureView> = 2;
template <> static VERA_CONSTEXPR uint64_t descriptor_idx_v<BufferView>  = 3;
template <> static VERA_CONSTEXPR uint64_t descriptor_idx_v<Buffer>      = 4;

// returns index corresponding to DescriptorWriteInfo variant index
// 0 -> Sampler
// 1 -> CombinedTextureSampler
// 2 -> TextureView
// 3 -> BufferView
// 4 -> Buffer
static size_t descriptor_type_to_index(DescriptorType type)
{
	switch (type) {
	case DescriptorType::Sampler:                            return 0;
	case DescriptorType::CombinedTextureSampler:             return 1;
	case DescriptorType::SampledTexture:                     return 2;
	case DescriptorType::StorageTexture:                     return 2;
	case DescriptorType::UniformTexelBuffer:                 return 3;
	case DescriptorType::StorageTexelBuffer:                 return 3;
	case DescriptorType::UniformBuffer:                      return 4;
	case DescriptorType::StorageBuffer:                      return 4;
	case DescriptorType::UniformBufferDynamic:               return 4;
	case DescriptorType::StorageBufferDynamic:               return 4;
	case DescriptorType::InputAttachment:                    return 2;
	// case DescriptorType::inlineUniformBlock:                 return -1;
	// case DescriptorType::AccelerationStructure:              return -1;
	// case DescriptorType::AccelerationStructureNV:            return -1;
	// case DescriptorType::PartitionedAccelerationStructureNV: return -1;
	}

	VERA_ASSERT_MSG(false, "invalid descriptor type");
	return {};
}

static DescriptorType get_binding_descriptor_type(DescriptorSetImpl& impl, uint32_t binding)
{
	return impl.descriptorSetLayout->getBinding(binding).descriptorType;
}

static size_t get_binding_type_index(DescriptorSetImpl& impl, uint32_t binding)
{
	return descriptor_type_to_index(get_binding_descriptor_type(impl, binding));
}

template <class ObjType>
static void store_binding_state(DescriptorSetImpl& impl, uint32_t binding, obj<ObjType> obj, uint32_t array_element)
{
	VERA_ASSERT_MSG(array_element < (1 << 24), "array_element cannot exceed 2^24 - 1");

	// make unique key for each binding state
	// key layout:
	// [ binding:32 | descriptor_idx:8 | array_element:24 ]

	uint64_t key = 
		static_cast<uint64_t>(binding) << 32 |
		descriptor_idx_v<ObjType> << 24 |
		static_cast<uint64_t>(array_element);

	impl.bindingStates[key] = obj_cast<CoreObject>(std::move(obj));
}

const vk::DescriptorSet& get_vk_descriptor_set(const_ref<DescriptorSet> descriptor_set) VERA_NOEXCEPT
{
	return CoreObject::getImpl(descriptor_set).vkDescriptorSet;
}

vk::DescriptorSet& get_vk_descriptor_set(ref<DescriptorSet> descriptor_set) VERA_NOEXCEPT
{
	return CoreObject::getImpl(descriptor_set).vkDescriptorSet;
}

DescriptorSet::~DescriptorSet() VERA_NOEXCEPT
{
	auto& impl      = getImpl(this);
	auto& pool_impl = getImpl(impl.descriptorPool);

	hash_t seed = 0;
	hash_combine(seed, static_cast<VkDescriptorSet>(impl.vkDescriptorSet));

	pool_impl.allocatedSets.erase(seed);

	if (pool_impl.flags.has(DescriptorPoolCreateFlagBits::FreeDescriptorSet)) {
		auto vk_device = get_vk_device(impl.device);
		vk_device.freeDescriptorSets(pool_impl.vkDescriptorPool, 1, &impl.vkDescriptorSet);
	}

	destroyObjectImpl(this);
}

void DescriptorSet::write(uint32_t binding, const DescriptorSamplerInfo& info, uint32_t array_element)
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::Sampler)
		throw Exception("descriptor type mismatch");

	vk::DescriptorImageInfo image_info;
	image_info.sampler = get_vk_sampler(info.sampler);
	store_binding_state(impl, binding, info.sampler, array_element);
	
	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = vk::DescriptorType::eSampler;
	write_info.descriptorCount = 1;
	write_info.pImageInfo      = &image_info;
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(uint32_t binding, array_view<DescriptorSamplerInfo> infos, uint32_t array_element)
{
	if (infos.empty()) return;
	
	if (infos.size() == 1) {
		write(binding, infos.front(), array_element);
		return;
	}

	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::Sampler)
		throw Exception("descriptor type mismatch");

	std::vector<vk::DescriptorImageInfo> image_infos;

	image_infos.reserve(infos.size());

	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = vk::DescriptorType::eSampler;
	write_info.descriptorCount = static_cast<uint32_t>(infos.size());
	write_info.pImageInfo      = image_infos.data();

	for (const auto& info : infos) {
		auto& image_info = image_infos.emplace_back();
		image_info.sampler = get_vk_sampler(info.sampler);
		store_binding_state(impl, binding, info.sampler, array_element++);
	}
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                                         binding,
	const DescriptorCombinedTextureSamplerInfo&      info,
	uint32_t                                         array_element
) {
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::CombinedTextureSampler)
		throw Exception("descriptor type mismatch");

	vk::DescriptorImageInfo image_info;
	image_info.sampler     = get_vk_sampler(info.sampler);
	image_info.imageView   = get_vk_image_view(info.textureView);
	image_info.imageLayout = to_vk_image_layout(info.layout);
	store_binding_state(impl, binding, info.sampler, array_element);
	store_binding_state(impl, binding, info.textureView, array_element);
	
	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
	write_info.descriptorCount = 1;
	write_info.pImageInfo      = &image_info;
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                                         binding,
	array_view<DescriptorCombinedTextureSamplerInfo> infos,
	uint32_t                                         array_element
) {
	if (infos.empty()) return;

	if (infos.size() == 1) {
		write(binding, infos.front(), array_element);
		return;
	}
	
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::CombinedTextureSampler)
		throw Exception("descriptor type mismatch");

	std::vector<vk::DescriptorImageInfo> image_infos;

	image_infos.reserve(infos.size());

	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = vk::DescriptorType::eCombinedImageSampler;
	write_info.descriptorCount = static_cast<uint32_t>(infos.size());
	write_info.pImageInfo      = image_infos.data();

	for (const auto& info : infos) {
		auto& image_info = image_infos.emplace_back();
		image_info.sampler     = get_vk_sampler(info.sampler);
		image_info.imageView   = get_vk_image_view(info.textureView);
		image_info.imageLayout = to_vk_image_layout(info.layout);
		store_binding_state(impl, binding, info.sampler, array_element);
		store_binding_state(impl, binding, info.textureView, array_element++);
	}
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                     binding,
	const DescriptorTextureInfo& info,
	uint32_t                     array_element
) {
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::SampledTexture &&
		desc_type != DescriptorType::StorageTexture &&
		desc_type != DescriptorType::InputAttachment)
		throw Exception("descriptor type mismatch");

	vk::DescriptorImageInfo image_info;
	image_info.imageView   = get_vk_image_view(info.textureView);
	image_info.imageLayout = to_vk_image_layout(info.layout);
	store_binding_state(impl, binding, info.textureView, array_element);
	
	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = to_vk_descriptor_type(desc_type);
	write_info.descriptorCount = 1;
	write_info.pImageInfo      = &image_info;
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                          binding,
	array_view<DescriptorTextureInfo> infos,
	uint32_t                          array_element
) {
	if (infos.empty()) return;

	if (infos.size() == 1) {
		write(binding, infos.front(), array_element);
		return;
	}
	
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::SampledTexture &&
		desc_type != DescriptorType::StorageTexture &&
		desc_type != DescriptorType::InputAttachment)
		throw Exception("descriptor type mismatch");

	std::vector<vk::DescriptorImageInfo> image_infos;

	image_infos.reserve(infos.size());

	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = to_vk_descriptor_type(desc_type);
	write_info.descriptorCount = static_cast<uint32_t>(infos.size());
	write_info.pImageInfo      = image_infos.data();

	for (const auto& info : infos) {
		auto& image_info = image_infos.emplace_back();
		image_info.imageView   = get_vk_image_view(info.textureView);
		image_info.imageLayout = to_vk_image_layout(info.layout);
		store_binding_state(impl, binding, info.textureView, array_element++);
	}
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                        binding,
	const DescriptorBufferViewInfo& info,
	uint32_t                        array_element
) {
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::UniformTexelBuffer &&
		desc_type != DescriptorType::StorageTexelBuffer)
		throw Exception("descriptor type mismatch");

	store_binding_state(impl, binding, info.bufferView, array_element);
	
	vk::WriteDescriptorSet write_info;
	write_info.dstSet           = impl.vkDescriptorSet;
	write_info.dstBinding       = binding;
	write_info.dstArrayElement  = array_element;
	write_info.descriptorType   = to_vk_descriptor_type(desc_type);
	write_info.descriptorCount  = 1;
	write_info.pTexelBufferView = &get_vk_buffer_view(info.bufferView);
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                           binding,
	array_view<DescriptorBufferViewInfo> infos,
	uint32_t                           array_element
) {
	if (infos.empty()) return;

	if (infos.size() == 1) {
		write(binding, infos.front(), array_element);
		return;
	}
	
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::UniformTexelBuffer &&
		desc_type != DescriptorType::StorageTexelBuffer)
		throw Exception("descriptor type mismatch");

	std::vector<vk::BufferView> buffer_views;

	buffer_views.reserve(infos.size());

	vk::WriteDescriptorSet write_info;
	write_info.dstSet           = impl.vkDescriptorSet;
	write_info.dstBinding       = binding;
	write_info.dstArrayElement  = array_element;
	write_info.descriptorType   = to_vk_descriptor_type(desc_type);
	write_info.descriptorCount  = static_cast<uint32_t>(buffer_views.size());
	write_info.pTexelBufferView = buffer_views.data();

	for (const auto& info : infos) {
		auto& buffer_view = buffer_views.emplace_back();
		buffer_view = get_vk_buffer_view(info.bufferView);
		store_binding_state(impl, binding, info.bufferView, array_element++);
	}
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                    binding,
	const DescriptorBufferInfo& info,
	uint32_t                    array_element
) {
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::UniformBuffer &&
		desc_type != DescriptorType::StorageBuffer &&
		desc_type != DescriptorType::UniformBufferDynamic &&
		desc_type != DescriptorType::StorageBufferDynamic)
		throw Exception("descriptor type mismatch");

	vk::DescriptorBufferInfo buffer_info;
	buffer_info.buffer = get_vk_buffer(info.buffer);
	buffer_info.offset = static_cast<vk::DeviceSize>(info.offset);
	buffer_info.range  = static_cast<vk::DeviceSize>(info.range);
	store_binding_state(impl, binding, info.buffer, array_element);
	
	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = to_vk_descriptor_type(desc_type);
	write_info.descriptorCount = 1;
	write_info.pBufferInfo     = &buffer_info;
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(
	uint32_t                         binding,
	array_view<DescriptorBufferInfo> infos,
	uint32_t                         array_element
) {
	if (infos.empty()) return;

	if (infos.size() == 1) {
		write(binding, infos.front(), array_element);
		return;
	}
	
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	auto  desc_type = get_binding_descriptor_type(impl, binding);

	if (desc_type != DescriptorType::UniformBuffer &&
		desc_type != DescriptorType::StorageBuffer &&
		desc_type != DescriptorType::UniformBufferDynamic &&
		desc_type != DescriptorType::StorageBufferDynamic)
		throw Exception("descriptor type mismatch");

	std::vector<vk::DescriptorBufferInfo> buffer_infos;

	buffer_infos.reserve(infos.size());

	vk::WriteDescriptorSet write_info;
	write_info.dstSet          = impl.vkDescriptorSet;
	write_info.dstBinding      = binding;
	write_info.dstArrayElement = array_element;
	write_info.descriptorType  = to_vk_descriptor_type(desc_type);
	write_info.descriptorCount = static_cast<uint32_t>(infos.size());
	write_info.pBufferInfo     = buffer_infos.data();

	for (const auto& info : infos) {
		auto& buffer_info = buffer_infos.emplace_back();
		buffer_info.buffer = get_vk_buffer(info.buffer);
		buffer_info.offset = static_cast<vk::DeviceSize>(info.offset);
		buffer_info.range  = static_cast<vk::DeviceSize>(info.range);
		store_binding_state(impl, binding, info.buffer, array_element++);
	}
	
	vk_device.updateDescriptorSets(1, &write_info, 0, nullptr);
}

void DescriptorSet::write(array_view<DescriptorWrite> writes)
{
	if (writes.empty()) return;

	auto& impl = getImpl(this);

	uint32_t prev_binding       = UINT32_MAX;
	uint32_t prev_array_element = UINT32_MAX;
	size_t   write_info_count   = 0;
	size_t   image_info_count   = 0;
	size_t   buffer_info_count  = 0;
	size_t   buffer_view_count  = 0;
	size_t   type_idx;

	for (const auto& write : writes) {
		if (write.binding != prev_binding)
			type_idx  = get_binding_type_index(impl, write.binding);

		size_t info_type = write.info.index();

		if (info_type != type_idx)
			throw Exception("descriptor type mismatch");

		switch (info_type) {
		case 0: // DescriptorSamplerInfo
		case 1: // DescriptorCombinedTextureSamplerInfo
		case 2: // DescriptorTextureInfo
			image_info_count++;
			break;
		case 3: // DescriptorBufferViewInfo
			buffer_view_count++;
			break;
		case 4: // DescriptorBufferInfo
			buffer_info_count++;
			break;
		default:
			throw Exception("invalid descriptor write info");
		}

		if (write.binding == prev_binding &&
			write.arrayElement == prev_array_element + 1 &&
			&write != &writes.back())
		{
			prev_array_element++;
			continue;
		}

		write_info_count++;

		prev_binding       = write.binding;
		prev_array_element = write.arrayElement;
	}

	std::vector<vk::WriteDescriptorSet>   write_infos;
	std::vector<vk::DescriptorImageInfo>  image_infos;
	std::vector<vk::DescriptorBufferInfo> buffer_infos;
	std::vector<vk::BufferView>           buffer_views;

	write_infos.reserve(write_info_count);
	image_infos.reserve(image_info_count);
	buffer_infos.reserve(buffer_info_count);
	buffer_views.reserve(buffer_view_count);

	uint32_t array_element_offset = 0;
	size_t   image_info_offset  = 0;
	size_t   buffer_info_offset = 0;
	size_t   buffer_view_offset = 0;

	prev_binding       = UINT32_MAX;
	prev_array_element = UINT32_MAX;

	for (const auto& write : writes) {
		size_t info_type = write.info.index();

		switch (info_type) {
		case 0: { // DescriptorSamplerInfo
			const auto& desc_info  = std::get<DescriptorSamplerInfo>(write.info);
			auto&       image_info = image_infos.emplace_back();
			image_info.sampler = get_vk_sampler(desc_info.sampler);
			store_binding_state(impl, write.binding, desc_info.sampler, write.arrayElement);
		} break;
		case 1: { // DescriptorCombinedTextureSamplerInfo
			const auto& desc_info  = std::get<DescriptorCombinedTextureSamplerInfo>(write.info);
			auto&       image_info = image_infos.emplace_back();
			image_info.sampler     = get_vk_sampler(desc_info.sampler);
			image_info.imageView   = get_vk_image_view(desc_info.textureView);
			image_info.imageLayout = to_vk_image_layout(desc_info.layout);
			store_binding_state(impl, write.binding, desc_info.sampler, write.arrayElement);
			store_binding_state(impl, write.binding, desc_info.textureView, write.arrayElement);
		} break;
		case 2: { // DescriptorTextureInfo
			const auto& desc_info  = std::get<DescriptorTextureInfo>(write.info);
			auto&       image_info = image_infos.emplace_back();
			image_info.imageView   = get_vk_image_view(desc_info.textureView);
			image_info.imageLayout = to_vk_image_layout(desc_info.layout);
			store_binding_state(impl, write.binding, desc_info.textureView, write.arrayElement);
		} break;
		case 3: { // DescriptorBufferViewInfo
			const auto& desc_info   = std::get<DescriptorBufferViewInfo>(write.info);
			auto&       buffer_view = buffer_views.emplace_back();
			buffer_view = get_vk_buffer_view(desc_info.bufferView);
			store_binding_state(impl, write.binding, desc_info.bufferView, write.arrayElement);
		} break;
		case 4: { // DescriptorBufferInfo
			const auto& desc_info   = std::get<DescriptorBufferInfo>(write.info);
			auto&       buffer_info = buffer_infos.emplace_back();
			buffer_info.buffer = get_vk_buffer(desc_info.buffer);
			buffer_info.offset = static_cast<vk::DeviceSize>(desc_info.offset);
			buffer_info.range  = static_cast<vk::DeviceSize>(desc_info.range);
			store_binding_state(impl, write.binding, desc_info.buffer, write.arrayElement);
		} break;
		}

		if (write.binding == prev_binding &&
			write.arrayElement == prev_array_element + 1 &&
			&write != &writes.back())
		{
			prev_array_element++;
			continue;
		}

		auto  desc_type  = get_binding_descriptor_type(impl, write.binding);
		auto& write_info = write_infos.emplace_back();
		write_info.dstSet          = get_vk_descriptor_set(ref<DescriptorSet>(this));
		write_info.dstBinding      = write.binding;
		write_info.dstArrayElement = write.arrayElement;

		switch (info_type) {
		case 0: // DescriptorSamplerInfo
		case 1: // DescriptorCombinedTextureSamplerInfo
		case 2: { // DescriptorTextureInfo
			write_info.descriptorCount = static_cast<uint32_t>(image_infos.size() - image_info_offset);
			write_info.descriptorType  = to_vk_descriptor_type(desc_type);
			write_info.pImageInfo      = &image_infos[image_info_offset];
		} break;
		case 3: { // DescriptorBufferViewInfo
			write_info.descriptorCount  = static_cast<uint32_t>(buffer_views.size() - buffer_view_offset);
			write_info.descriptorType   = to_vk_descriptor_type(desc_type);
			write_info.pTexelBufferView = &buffer_views[buffer_view_offset];
		} break;
		case 4: { // DescriptorBufferInfo
			write_info.descriptorCount = static_cast<uint32_t>(buffer_infos.size() - buffer_info_offset);
			write_info.descriptorType  = to_vk_descriptor_type(desc_type);
			write_info.pBufferInfo     = &buffer_infos[buffer_info_offset];
		} break;
		}

		prev_binding       = write.binding;
		prev_array_element = write.arrayElement;
	}

	auto vk_device = get_vk_device(impl.device);
	
	vk_device.updateDescriptorSets(
		static_cast<uint32_t>(write_infos.size()),
		write_infos.data(),
		0,
		nullptr
	);
}

obj<Device> DescriptorSet::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

obj<DescriptorPool> DescriptorSet::getDescriptorPool() VERA_NOEXCEPT
{
	return getImpl(this).descriptorPool;
}

obj<DescriptorSetLayout> DescriptorSet::getDescriptorSetLayout() VERA_NOEXCEPT
{
	return getImpl(this).descriptorSetLayout;
}

bool DescriptorSet::isValid() const VERA_NOEXCEPT
{
	return static_cast<bool>(getImpl(this).vkDescriptorSet);
}

VERA_NAMESPACE_END