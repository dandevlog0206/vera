#pragma once

#include "core_object.h"
#include "../util/flag.h"

VERA_NAMESPACE_BEGIN

class Device;

enum class MemoryPropertyFlagBits VERA_FLAG_BITS
{
	DeviceLocal     = 1 << 0,
	HostVisible     = 1 << 1,
	HostCoherent    = 1 << 2,
	HostCached      = 1 << 3,
	LazilyAllocated = 1 << 4,
	Protected       = 1 << 5
} VERA_ENUM_FLAGS(MemoryPropertyFlagBits, MemoryPropertyFlags)

enum class AccessFlagBits VERA_FLAG_BITS
{
	IndirectCommandRead         = 1 << 0,
	IndexRead                   = 1 << 1,
	VertexAttributeRead         = 1 << 2,
	UniformRead                 = 1 << 3,
	InputAttachmentRead         = 1 << 4,
	ShaderRead                  = 1 << 5,
	ShaderWrite                 = 1 << 6,
	ColorAttachmentRead         = 1 << 7,
	ColorAttachmentWrite        = 1 << 8,
	DepthStencilAttachmentRead  = 1 << 9,
	DepthStencilAttachmentWrite = 1 << 10,
	TransferRead                = 1 << 11,
	TransferWrite               = 1 << 12,
	HostRead                    = 1 << 13,
	HostWrite                   = 1 << 14,
	MemoryRead                  = 1 << 15,
	MemoryWrite                 = 1 << 16,
	None                        = 1 << 17,
} VERA_ENUM_FLAGS(AccessFlagBits, AccessFlags)

struct DeviceMemoryCreateInfo
{
	size_t              size;
	MemoryPropertyFlags memoryProperties;
};

class DeviceMemory : public CoreObject
{
	VERA_CORE_OBJECT(DeviceMemory)
public:
	static ref<DeviceMemory> create(ref<Device> device, const DeviceMemoryCreateInfo& info);
	~DeviceMemory();

	void resize(size_t new_size, bool keep_contents = false);

	void* map();
	void unmap();
	void flush();
};

VERA_NAMESPACE_END