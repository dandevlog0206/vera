#include "../../include/vera/core/buffer.h"
#include "../impl/buffer_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/device_memory.h"

VERA_NAMESPACE_BEGIN

ref<Buffer> Buffer::create(ref<Device> device, const BufferCreateInfo& info)
{
	auto  obj       = createNewObject<Buffer>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	// TODO: implement

	vk::BufferCreateInfo buffer_info;
	buffer_info.size        = info.size;
	buffer_info.usage       = to_vk_buffer_usage_flags(info.usage);
	buffer_info.sharingMode = vk::SharingMode::eExclusive;

	impl.device = std::move(device);
	impl.buffer = vk_device.createBuffer(buffer_info);

	return obj;
}

ref<Buffer> Buffer::create(ref<DeviceMemory> memory, size_t offset, const BufferCreateInfo& info)
{
	auto obj   = createNewObject<Buffer>();
	auto& impl = getImpl(obj);
	// TODO: implement

	return obj;
}

Buffer::~Buffer()
{
	// TODO: implement
}

VERA_NAMESPACE_END
