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
	VERA_CORE_OBJECT(Buffer)
public:
	static ref<Buffer> createVertex(ref<Device> device, size_t size);
	static ref<Buffer> createVertex(ref<DeviceMemory> memory, size_t offset, size_t size);
	static ref<Buffer> createIndex(ref<Device> device, IndexType type, size_t count);
	static ref<Buffer> createIndex(ref<DeviceMemory> memory, size_t offset, IndexType type, size_t count);
	static ref<Buffer> createStaging(ref<Device> device, size_t size);
	static ref<Buffer> create(ref<Device> device, const BufferCreateInfo& info);
	static ref<Buffer> create(ref<DeviceMemory> memory, size_t offset, const BufferCreateInfo& info);
	~Buffer();

	void resize(size_t size);

	ref<DeviceMemory> getDeviceMemory();

	BufferUsageFlags getUsageFlags() const;

	size_t size() const;
};

VERA_NAMESPACE_END