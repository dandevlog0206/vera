#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class RenderCommand;
class Fence;
class Semaphore;
class Texture;

struct RenderFrame
{
	ref<RenderCommand> renderCommand;
	ref<Fence>         fence;
	ref<Semaphore>     renderCompleteSemaphore;
	ref<Semaphore>     imageWaitSemaphore;
	ref<Texture>       swapchainImage;
	bool               isBegin;
	bool               isSubmitted;
};

struct RenderContextImpl
{
	ref<Device>              device;

	std::vector<RenderFrame> renderFrames;
	int32_t                  frameIndex;
	int32_t                  lastFrameIndex;
	uint64_t                 totalFrameCount;
};

VERA_NAMESPACE_END