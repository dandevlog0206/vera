#pragma once

#include "object_impl.h"
#include "../../include/vera/core/render_frame.h"

VERA_NAMESPACE_BEGIN

class CommandBuffer;
class Fence;
class Semaphore;
class Texture;

struct RenderContextFrame : RenderFrame
{
	using FrameBuffers = std::vector<ref<FrameBuffer>>;

	FrameBuffers framebuffers;
};

struct RenderContextImpl
{
	using RenderContextFrames = std::vector<RenderContextFrame*>;

	obj<Device>         device;

	RenderContextFrames renderFrames;
	int32_t             frameIndex;
	uint64_t            currentFrameID;
	bool                dynamicFrameCount;
};

VERA_NAMESPACE_END