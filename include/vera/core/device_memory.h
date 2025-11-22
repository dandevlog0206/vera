#pragma once

#include "device.h"
#include "../util/array_view.h"
#include <bitset>

VERA_NAMESPACE_BEGIN

class Buffer;
class Texture;

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
	~DeviceMemory() VERA_NOEXCEPT override;

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