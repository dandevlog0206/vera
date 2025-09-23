#pragma once

#include "buffer.h"

VERA_NAMESPACE_BEGIN

struct BufferViewCreateInfo
{
	size_t size;
};

class BufferView : protected CoreObject
{
	VERA_CORE_OBJECT(BufferView)
public:
	ref<BufferView> create(ref<BufferView> device, const BufferViewCreateInfo& info);
	~BufferView();
};

VERA_NAMESPACE_END