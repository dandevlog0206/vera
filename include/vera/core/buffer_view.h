#pragma once

#include "buffer.h"

VERA_NAMESPACE_BEGIN

class Device;
class Buffer;

struct BufferViewCreateInfo
{
	Format format;
	size_t offset;
	size_t size;
};

class BufferView : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(BufferView)
public:
	obj<BufferView> create(obj<Buffer> buffer, const BufferViewCreateInfo& info);
	~BufferView();

	obj<Device> getDevice();
	obj<Buffer> getBuffer();

	Format getFormat() const;

	size_t size();
	size_t offset();
};

VERA_NAMESPACE_END