#pragma once

#include "buffer.h"

VERA_NAMESPACE_BEGIN

struct BufferViewCreateInfo
{
	size_t size;
};

class BufferView : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(BufferView)
public:
	obj<BufferView> create(obj<BufferView> device, const BufferViewCreateInfo& info);
	~BufferView();

	obj<Device> getDevice();
	obj<Buffer> getBuffer();
};

VERA_NAMESPACE_END