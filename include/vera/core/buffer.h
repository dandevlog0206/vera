#pragma once

#include "device.h"
#include "../util/flag.h"

VERA_NAMESPACE_BEGIN

class Device;
class DeviceMemory;

enum class IndexType VERA_ENUM
{
	Unknown,
	UInt8,
	UInt16,
	UInt32
};

enum class BufferUsageFlagBits VERA_FLAG_BITS
{
	TransferSrc        = 1 << 0,
	TransferDst        = 1 << 1,
	UniformTexelBuffer = 1 << 2,
	StorageTexelBuffer = 1 << 3,
	UniformBuffer      = 1 << 4,
	StorageBuffer      = 1 << 5,
	IndexBuffer        = 1 << 6,
	VertexBuffer       = 1 << 7,
	IndirectBuffer     = 1 << 8
} VERA_ENUM_FLAGS(BufferUsageFlagBits, BufferUsageFlags)

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