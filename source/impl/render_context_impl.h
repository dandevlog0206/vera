#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class CommandBuffer;
class Fence;
class Semaphore;
class Texture;
struct RenderFrame;

class FrameSync
{
public:
	FrameSync() = default;
	FrameSync(const RenderFrame& ctx_impl, uint64_t frame_id);

	void waitForRenderComplete() const;
	bool isRenderComplete() const;
	ref<Semaphore> getRenderCompleteSemaphore() const;

	bool empty() const;

private:
	const RenderFrame* m_render_frame;
	uint64_t           m_frame_id;
};

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