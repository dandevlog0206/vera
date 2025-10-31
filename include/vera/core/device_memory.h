#pragma once

#include "device.h"
#include "../util/flag.h"
#include "../util/array_view.h"
#include <bitset>

VERA_NAMESPACE_BEGIN

class Device;
class Buffer;
class Texture;

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
	MemoryPropertyFlags propertyFlags;
	std::bitset<32>     memoryTypeMask;
};

class DeviceMemory : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(DeviceMemory)
public:
	static obj<DeviceMemory> create(obj<Device> device, const DeviceMemoryCreateInfo& info);
	~DeviceMemory();

	void resize(size_t new_size, bool keep_contents = false);

	// TODO: consider using ref<>
	void bindBuffer(obj<Buffer> buffer, size_t offset);
	void bindTexture(obj<Texture> texture, size_t offset);

	VERA_NODISCARD void* map();
	void unmap();
	void flush();

	template <class T>
	void upload(array_view<T> arr, size_t offset = 0);
	void upload(const void* data, size_t size, size_t offset = 0);
};

template <class T>
void DeviceMemory::upload(array_view<T> arr, size_t offset)
{
	upload(arr.data(), arr.size() * sizeof(T), offset);
}

VERA_NAMESPACE_END