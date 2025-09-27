#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class RenderCommand;
class Fence;
class Semaphore;
class Texture;

struct RenderFrame
{
	obj<RenderCommand> renderCommand;
	obj<Fence>         fence;
	obj<Semaphore>     renderCompleteSemaphore;
	obj<Semaphore>     imageWaitSemaphore;
	ref<Texture>       swapchainImage;
	bool               isBegin;
	bool               isSubmitted;
};

struct RenderContextImpl
{
	obj<Device>              device;

	std::vector<RenderFrame> renderFrames;
	int32_t                  frameIndex;
	int32_t                  lastFrameIndex;
	uint64_t                 totalFrameCount;
};

VERA_NAMESPACE_END