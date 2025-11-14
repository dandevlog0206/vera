#pragma once

#include "device_memory.h"

VERA_NAMESPACE_BEGIN

struct BufferCreateInfo
{
	size_t              size;
	BufferUsageFlags    usage;
	MemoryPropertyFlags propetyFlags;
};

class Buffer : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Buffer)
public:
	static obj<Buffer> createVertex(obj<Device> device, size_t size);
	static obj<Buffer> createVertex(obj<DeviceMemory> memory, size_t offset, size_t size);
	static obj<Buffer> createIndex(obj<Device> device, IndexType type, size_t count);
	static obj<Buffer> createIndex(obj<DeviceMemory> memory, size_t offset, IndexType type, size_t count);
	static obj<Buffer> createUniform(obj<Device> device, size_t size);
	static obj<Buffer> createStorage(obj<Device> device, size_t size);
	static obj<Buffer> createStaging(obj<Device> device, size_t size);
	static obj<Buffer> create(obj<Device> device, const BufferCreateInfo& info);
	static obj<Buffer> create(obj<DeviceMemory> memory, size_t offset, const BufferCreateInfo& info);
	~Buffer();

	void resize(size_t new_size);

	obj<DeviceMemory> getDeviceMemory();

	BufferUsageFlags getUsageFlags() const;

	size_t size() const;
};

VERA_NAMESPACE_END