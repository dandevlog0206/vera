#include "../../include/vera/core/buffer.h"
#include "../impl/buffer_impl.h"
#include "../impl/device_impl.h"
#include "../impl/device_memory_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/device_memory.h"

VERA_NAMESPACE_BEGIN

static size_t get_index_size(IndexType type)
{
	switch (type) {
	case IndexType::UInt8:	return 1;
	case IndexType::UInt16:	return 2;
	case IndexType::UInt32: return 4;
	}
	
	VERA_ASSERT_MSG(false, "invalid index type");
	return 0;
}

static void allocate_device_memory(
	DeviceMemoryImpl&       impl,
	ref<Device>             device,
	vk::Buffer              buffer,
	vk::MemoryPropertyFlags flags
) {
	auto& device_impl = CoreObject::getImpl(device);
	auto  req         = device_impl.device.getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo info;
	info.allocationSize  = req.size;
	info.memoryTypeIndex = get_memory_type_index(device_impl, req.memoryTypeBits, flags);

	impl.device        = std::move(device);
	impl.memory        = device_impl.device.allocateMemory(info);
	impl.propertyFlags = flags;
	impl.typeIndex     = info.memoryTypeIndex;
	impl.mapPtr        = nullptr;
}

ref<Buffer> Buffer::createVertex(ref<Device> device, size_t size)
{
	BufferCreateInfo info;
	info.size         = size;
	info.usage        =
		BufferUsageFlagBits::VertexBuffer |
		BufferUsageFlagBits::TransferDst |
		BufferUsageFlagBits::TransferSrc;
	info.propetyFlags = 
		MemoryPropertyFlagBits::DeviceLocal |
		MemoryPropertyFlagBits::HostVisible |
		MemoryPropertyFlagBits::HostCoherent;

	return create(device, info);
}

ref<Buffer> Buffer::createVertex(ref<DeviceMemory> memory, size_t offset, size_t size)
{
	BufferCreateInfo info;
	info.size         = size;
	info.usage        =
		BufferUsageFlagBits::VertexBuffer |
		BufferUsageFlagBits::TransferDst |
		BufferUsageFlagBits::TransferSrc;
	info.propetyFlags = 
		MemoryPropertyFlagBits::DeviceLocal |
		MemoryPropertyFlagBits::HostVisible |
		MemoryPropertyFlagBits::HostCoherent;

	return create(memory, offset, info);
}

ref<Buffer> Buffer::createIndex(ref<Device> device, IndexType type, size_t count)
{
	BufferCreateInfo info;
	info.size         = get_index_size(type) * count;
	info.usage        =
		BufferUsageFlagBits::IndexBuffer |
		BufferUsageFlagBits::TransferDst |
		BufferUsageFlagBits::TransferSrc;
	info.propetyFlags = 
		MemoryPropertyFlagBits::DeviceLocal |
		MemoryPropertyFlagBits::HostVisible |
		MemoryPropertyFlagBits::HostCoherent;

	auto  obj  = create(device, info);
	auto& impl = getImpl(obj);

	impl.indexType = type;

	return obj;
}

ref<Buffer> Buffer::createIndex(ref<DeviceMemory> memory, size_t offset, IndexType type, size_t count)
{
	BufferCreateInfo info;
	info.size         = get_index_size(type) * count;
	info.usage        =
		BufferUsageFlagBits::IndexBuffer |
		BufferUsageFlagBits::TransferDst |
		BufferUsageFlagBits::TransferSrc;
	info.propetyFlags = 
		MemoryPropertyFlagBits::DeviceLocal |
		MemoryPropertyFlagBits::HostVisible |
		MemoryPropertyFlagBits::HostCoherent;

	auto  obj  = create(memory, offset, info);
	auto& impl = getImpl(obj);

	impl.indexType = type;

	return obj;
}

ref<Buffer> Buffer::createStaging(ref<Device> device, size_t size)
{
	BufferCreateInfo info;
	info.size         = size;
	info.usage        =
		BufferUsageFlagBits::TransferDst |
		BufferUsageFlagBits::TransferSrc;
	info.propetyFlags = 
		MemoryPropertyFlagBits::DeviceLocal |
		MemoryPropertyFlagBits::HostVisible |
		MemoryPropertyFlagBits::HostCoherent;

	return create(device, info);
}

ref<Buffer> Buffer::create(ref<Device> device, const BufferCreateInfo& info)
{
	auto  obj           = createNewObject<Buffer>();
	auto  memory_obj    = createNewObject<DeviceMemory>();
	auto& impl          = getImpl(obj);
	auto& memory_impl   = getImpl(memory_obj);
	auto& device_impl   = getImpl(device);
	auto property_flags = to_vk_memory_property_flags(info.propetyFlags);

	vk::BufferCreateInfo buffer_info;
	buffer_info.size        = info.size;
	buffer_info.usage       = to_vk_buffer_usage_flags(info.usage);
	buffer_info.sharingMode = vk::SharingMode::eExclusive;

	impl.device = device;
	impl.memory = std::move(memory_obj);
	impl.buffer = device_impl.device.createBuffer(buffer_info);
	impl.usage  = info.usage;
	impl.size   = info.size;

	allocate_device_memory(memory_impl, std::move(device), impl.buffer, property_flags);

	auto& binding = memory_impl.resourceBind.emplace_back();
	binding.resourceType = MemoryResourceType::Buffer;
	binding.size         = info.size;
	binding.offset       = 0;
	binding.resourcePtr  = obj.get();

	device_impl.device.bindBufferMemory(impl.buffer, memory_impl.memory, 0);

	return obj;
}

ref<Buffer> Buffer::create(ref<DeviceMemory> memory, size_t offset, const BufferCreateInfo& info)
{
	auto  obj         = createNewObject<Buffer>();
	auto& impl        = getImpl(obj);
	auto& memory_impl = getImpl(memory);
	auto& device_impl = getImpl(memory_impl.device);
	
	vk::BufferCreateInfo buffer_info;
	buffer_info.size        = info.size;
	buffer_info.usage       = to_vk_buffer_usage_flags(info.usage);
	buffer_info.sharingMode = vk::SharingMode::eExclusive;

	impl.device = memory_impl.device;
	impl.memory = std::move(memory);
	impl.buffer = device_impl.device.createBuffer(buffer_info);
	impl.usage  = info.usage;
	impl.size   = info.size;

	auto& binding = memory_impl.resourceBind.emplace_back();
	binding.resourceType = MemoryResourceType::Buffer;
	binding.size         = info.size;
	binding.offset       = offset;
	binding.resourcePtr  = obj.get();

	device_impl.device.bindBufferMemory(impl.buffer, memory_impl.memory, offset);

	return obj;
}

Buffer::~Buffer()
{
	auto& impl          = getImpl(this);
	auto& resource_bind = getImpl(impl.memory).resourceBind;
	auto  vk_device     = get_vk_device(impl.device);
	
	auto iter = std::find_if(VERA_SPAN(resource_bind), [this](const auto& bind) {
		return bind.resourcePtr == this;
	});

	VERA_ASSERT(iter != resource_bind.end());

	std::iter_swap(iter, resource_bind.end() - 1);
	resource_bind.pop_back();
	
	vk_device.destroy(impl.buffer);

	destroyObjectImpl(this);
}

void Buffer::resize(size_t size)
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
	auto& memory_impl = getImpl(impl.memory);

	// TODO: implement
}

ref<DeviceMemory> Buffer::getDeviceMemory()
{
	return getImpl(this).memory;
}

BufferUsageFlags Buffer::getUsageFlags() const
{
	return getImpl(this).usage;
}

size_t Buffer::size() const
{
	return getImpl(this).size;
}

VERA_NAMESPACE_END
