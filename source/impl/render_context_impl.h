#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class CommandBuffer;
class Fence;
class Semaphore;
class Texture;

struct RenderFrame
{
	using FrameBuffers = std::vector<ref<FrameBuffer>>;

	obj<CommandBuffer> renderCommand;
	FrameBuffers       framebuffers;
	obj<Semaphore>     renderCompleteSemaphore;
	obj<Fence>         fence;
	uint64_t           frameID;
};

struct RenderContextImpl
{
	using RenderFrames = std::vector<RenderFrame*>;

	obj<Device>  device;

	RenderFrames renderFrames;
	int32_t      frameIndex;
	uint64_t     currentFrameID;
};

VERA_NAMESPACE_END