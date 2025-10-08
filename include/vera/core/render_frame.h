#pragma once

#include "command_buffer_sync.h"

VERA_NAMESPACE_BEGIN

class CommandBuffer;

struct RenderFrame
{
	obj<CommandBuffer> commandBuffer;
	CommandBufferSync  commandBufferSync;
	uint32_t           frameIndex;
	uint64_t           frameID;
};

VERA_NAMESPACE_END
