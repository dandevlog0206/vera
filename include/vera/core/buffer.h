#pragma once

#include "core_object.h"
#include "../util/flag.h"

VERA_NAMESPACE_BEGIN

class Device;
class DeviceMemory;

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
	size_t           size;
	BufferUsageFlags usage;
};

class Buffer : protected CoreObject
{
	VERA_CORE_OBJECT(Buffer)
public:
	ref<Buffer> create(ref<Device> device, const BufferCreateInfo& info);
	ref<Buffer> create(ref<DeviceMemory> memory, size_t offset, const BufferCreateInfo& info);
	~Buffer();
};

VERA_NAMESPACE_END