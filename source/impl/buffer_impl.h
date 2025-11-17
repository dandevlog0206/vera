#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class BufferImpl
{
public:
	obj<Device>       device    = {};
	obj<DeviceMemory> memory    = {};

	vk::Buffer        vkBuffer  = {};

	size_t            size      = {};
	BufferUsageFlags  usage     = {};
	IndexType         indexType = {};
};

struct BufferViewImpl
{
	obj<Device>    device       = {};
	obj<Buffer>    buffer       = {};

	vk::BufferView vkBufferView = {};

	Format         format       = {};
	size_t         offset       = {};
	size_t         size         = {};
};

VERA_NAMESPACE_END