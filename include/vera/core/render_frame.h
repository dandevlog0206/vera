#pragma once

#include "command_sync.h"

VERA_NAMESPACE_BEGIN

class CommandBuffer;

struct RenderFrame
{
	obj<CommandBuffer> commandBuffer;
	CommandSync        sync;
	uint32_t           frameIndex;
	uint64_t           frameID;
};

VERA_NAMESPACE_END
