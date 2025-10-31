#include "../../include/vera/core/device_memory.h"
#include "../impl/device_impl.h"
#include "../impl/device_memory_impl.h"
#include "../impl/buffer_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/texture.h"

VERA_NAMESPACE_BEGIN

obj<DeviceMemory> DeviceMemory::create(obj<Device> device, const DeviceMemoryCreateInfo& info)
{
	auto  obj         = createNewCoreObject<DeviceMemory>();
	auto& impl        = getImpl(obj);
	auto& device_impl = getImpl(device);

	auto type_idx = find_memory_type_idx(device_impl, info.propertyFlags, info.memoryTypeMask);

	vk::MemoryAllocateInfo alloc_info;
	alloc_info.allocationSize  = info.size;
	alloc_info.memoryTypeIndex = type_idx;

	impl.device        = std::move(device);
	impl.memory        = device_impl.device.allocateMemory(alloc_info);
	impl.propertyFlags = info.propertyFlags;
	impl.allocated     = info.size;
	impl.typeIndex     = alloc_info.memoryTypeIndex;
	impl.mapPtr        = nullptr;

	return obj;
}

DeviceMemory::~DeviceMemory()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.device.freeMemory(impl.memory);

	destroyObjectImpl(this);
}

void DeviceMemory::resize(size_t new_size, bool keep_contents)
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	// TODO: implement DeviceMemory::resize() keep contents

	vk_device.free(impl.memory);
	impl.allocated = 0;
	impl.mapPtr    = nullptr;

	vk::MemoryAllocateInfo alloc_info;
	alloc_info.allocationSize  = new_size;
	alloc_info.memoryTypeIndex = impl.typeIndex;

	impl.memory    = vk_device.allocateMemory(alloc_info);
	impl.allocated = new_size;

	// Memory lacking will be captured by validation layer
	for (const auto& binding : impl.resourceBind) {
		switch (binding.resourceType) {
		case MemoryResourceType::Buffer: {
			auto& buffer_impl = getImpl(reinterpret_cast<Buffer*>(binding.resourcePtr));
			vk_device.bindBufferMemory(buffer_impl.buffer, impl.memory, binding.offset);
		} break;
		case MemoryResourceType::Texture: {
			auto& texture_impl = getImpl(reinterpret_cast<Texture*>(binding.resourcePtr));
			vk_device.bindImageMemory(texture_impl.image, impl.memory, binding.offset);
		} break;
		default:
			throw Exception("invalid memory resource type");
		}
	}
}

void DeviceMemory::bindBuffer(obj<Buffer> buffer, size_t offset)
{
	auto& impl        = getImpl(this);
	auto& buffer_impl = getImpl(buffer);
	auto  vk_device   = get_vk_device(impl.device);

	vk_device.bindBufferMemory(buffer_impl.buffer, impl.memory, offset);

	for (auto& binding : impl.resourceBind) {
		if (binding.resourcePtr == buffer.get()) {
			binding.offset = offset;
			return;
		}
	}

	auto& binding = impl.resourceBind.emplace_back();
	binding.resourceType = MemoryResourceType::Buffer;
	binding.size         = buffer_impl.size;
	binding.offset       = offset;
	binding.resourcePtr  = buffer.get();
}

void DeviceMemory::bindTexture(obj<Texture> texture, size_t offset)
{
	auto& impl         = getImpl(this);
	auto& texture_impl = getImpl(texture);
	auto  vk_device    = get_vk_device(impl.device);

	vk_device.bindImageMemory(texture_impl.image, impl.memory, offset);

	for (auto& binding : impl.resourceBind) {
		if (binding.resourcePtr == texture.get()) {
			binding.offset = offset;
			return;
		}
	}

	auto& binding = impl.resourceBind.emplace_back();
	binding.resourceType = MemoryResourceType::Texture;
	binding.size         = texture_impl.size;
	binding.offset       = offset;
	binding.resourcePtr  = texture.get();
}

void* DeviceMemory::map()
{
	auto& impl = getImpl(this);
	
	if (impl.mapPtr) 
		return impl.mapPtr;

	if (!impl.propertyFlags.has(MemoryPropertyFlagBits::HostVisible))
		throw Exception("attempt to map non-host-visible memory");

	auto vk_device = get_vk_device(impl.device);

	return impl.mapPtr = vk_device.mapMemory(impl.memory, 0, VK_WHOLE_SIZE);
}

void DeviceMemory::unmap()
{
	auto& impl = getImpl(this);

	if (!impl.mapPtr) return;

	auto vk_device = get_vk_device(impl.device);

	vk_device.unmapMemory(impl.memory);
	impl.mapPtr = nullptr;
}

void DeviceMemory::flush()
{
	auto& impl = getImpl(this);

	if (!impl.mapPtr) return;

	auto vk_device = get_vk_device(impl.device);

	vk::MappedMemoryRange range;
	range.memory = impl.memory;
	range.offset = 0;
	range.size   = VK_WHOLE_SIZE;

	vk_device.flushMappedMemoryRanges(range);
}

void DeviceMemory::upload(const void* data, size_t size, size_t offset)
{
	auto& impl = getImpl(this);
	void* dst  = reinterpret_cast<uint8_t*>(map()) + offset;

	if (impl.allocated < offset + size)
		throw Exception("attempt to upload more data than allocated memory size");

	auto  vk_device = get_vk_device(impl.device);
	
	vk::MappedMemoryRange range;
	range.memory = impl.memory;
	range.offset = offset;
	range.size   = size;

	std::memcpy(dst, data, size);
	vk_device.flushMappedMemoryRanges(range);
}

VERA_NAMESPACE_END