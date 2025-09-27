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

static uint32_t find_buffer_bind_idx(DeviceMemoryImpl& impl, Buffer* this_ptr)
{
	auto iter = std::find_if(VERA_SPAN(impl.resourceBind),
		[=](const auto& bind) {
			return bind.resourcePtr == this_ptr;
		});

	VERA_ASSERT(iter != impl.resourceBind.end());

	return iter - impl.resourceBind.cbegin();
}

static void allocate_device_memory(
	DeviceMemoryImpl&   impl,
	obj<Device>         device,
	vk::Buffer          buffer,
	MemoryPropertyFlags flags
) {
	auto& device_impl = CoreObject::getImpl(device);
	auto  req         = device_impl.device.getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo info;
	info.allocationSize  = req.size;
	info.memoryTypeIndex = find_memory_type_idx(device_impl, flags, req.memoryTypeBits);

	impl.device        = std::move(device);
	impl.memory        = device_impl.device.allocateMemory(info);
	impl.propertyFlags = flags;
	impl.allocated     = info.allocationSize;
	impl.typeIndex     = info.memoryTypeIndex;
	impl.mapPtr        = nullptr;
}

obj<Buffer> Buffer::createVertex(obj<Device> device, size_t size)
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

obj<Buffer> Buffer::createVertex(obj<DeviceMemory> memory, size_t offset, size_t size)
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

obj<Buffer> Buffer::createIndex(obj<Device> device, IndexType type, size_t count)
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

obj<Buffer> Buffer::createIndex(obj<DeviceMemory> memory, size_t offset, IndexType type, size_t count)
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

obj<Buffer> Buffer::createStaging(obj<Device> device, size_t size)
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

obj<Buffer> Buffer::create(obj<Device> device, const BufferCreateInfo& info)
{
	auto  obj           = createNewObject<Buffer>();
	auto  memory_obj    = createNewObject<DeviceMemory>();
	auto& impl          = getImpl(obj);
	auto& memory_impl   = getImpl(memory_obj);
	auto& device_impl   = getImpl(device);

	vk::BufferCreateInfo buffer_info;
	buffer_info.size        = info.size;
	buffer_info.usage       = to_vk_buffer_usage_flags(info.usage);
	buffer_info.sharingMode = vk::SharingMode::eExclusive;

	impl.device = device;
	impl.memory = std::move(memory_obj);
	impl.buffer = device_impl.device.createBuffer(buffer_info);
	impl.size   = info.size;
	impl.usage  = info.usage;

	allocate_device_memory(memory_impl, std::move(device), impl.buffer, info.propetyFlags);

	auto& binding = memory_impl.resourceBind.emplace_back();
	binding.resourceType = MemoryResourceType::Buffer;
	binding.size         = info.size;
	binding.offset       = 0;
	binding.resourcePtr  = obj.get();

	device_impl.device.bindBufferMemory(impl.buffer, memory_impl.memory, 0);

	return obj;
}

obj<Buffer> Buffer::create(obj<DeviceMemory> memory, size_t offset, const BufferCreateInfo& info)
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
	impl.size   = info.size;
	impl.usage  = info.usage;

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
	auto& impl        = getImpl(this);
	auto& memory_impl = getImpl(impl.memory);
	auto  vk_device   = get_vk_device(impl.device);
	
	auto idx = find_buffer_bind_idx(memory_impl, this);

	std::swap(memory_impl.resourceBind[idx], memory_impl.resourceBind.back());
	memory_impl.resourceBind.pop_back();
	
	vk_device.destroy(impl.buffer);

	destroyObjectImpl(this);
}

void Buffer::resize(size_t new_size)
{
	auto& impl = getImpl(this);

	if (impl.size == new_size) return;

	auto& memory_impl = getImpl(impl.memory);
	auto  vk_device   = get_vk_device(impl.device);
	auto  idx         = find_buffer_bind_idx(memory_impl, this);
	auto& binding     = memory_impl.resourceBind[idx];

	vk::BufferCreateInfo buffer_info;
	buffer_info.size        = impl.size;
	buffer_info.usage       = to_vk_buffer_usage_flags(impl.usage);
	buffer_info.sharingMode = vk::SharingMode::eExclusive;

	vk_device.destroy(impl.buffer);
	impl.buffer = vk_device.createBuffer(buffer_info);
	impl.size   = new_size;

	if (memory_impl.allocated < binding.offset + new_size)
		impl.memory->resize(binding.offset + new_size);

	binding.size = new_size;
}

obj<DeviceMemory> Buffer::getDeviceMemory()
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
