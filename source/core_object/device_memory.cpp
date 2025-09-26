#include "../../include/vera/core/device_memory.h"
#include "../impl/device_impl.h"
#include "../impl/device_memory_impl.h"

#include "../../include/vera/core/device.h"

VERA_NAMESPACE_BEGIN

ref<DeviceMemory> DeviceMemory::create(ref<Device> device, const DeviceMemoryCreateInfo& info)
{
	auto  obj         = createNewObject<DeviceMemory>();
	auto& impl        = getImpl(obj);
	auto& device_impl = getImpl(device);

	vk::MemoryAllocateInfo allocate_info;
	allocate_info.allocationSize  = info.size;
	allocate_info.memoryTypeIndex = {};

	impl.device = std::move(device);
	impl.memory = device_impl.device.allocateMemory(allocate_info);
	impl.mapPtr = nullptr;

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
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
}

void* DeviceMemory::map()
{
	auto& impl = getImpl(this);
	
	if (impl.mapPtr) 
		return impl.mapPtr;

	auto& device_impl = getImpl(impl.device);

	return impl.mapPtr = device_impl.device.mapMemory(impl.memory, 0, VK_WHOLE_SIZE);
}

void DeviceMemory::unmap()
{
	auto& impl = getImpl(this);

	if (!impl.mapPtr) return;

	auto& device_impl = getImpl(impl.device);

	device_impl.device.unmapMemory(impl.memory);
	impl.mapPtr = nullptr;
}

void DeviceMemory::flush()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	vk::MappedMemoryRange range;
	range.memory = impl.memory;
	range.offset = 0;
	range.size   = VK_WHOLE_SIZE;

	device_impl.device.flushMappedMemoryRanges(range);
}

VERA_NAMESPACE_END