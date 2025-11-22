#pragma once

#include "object_impl.h"
#include "../../include/vera/core/render_frame.h"

VERA_NAMESPACE_BEGIN

struct RenderContextFrame : RenderFrame
{
	using FrameBuffers = std::vector<ref<FrameBuffer>>;

	FrameBuffers framebuffers;
};

class RenderContextImpl
{
public:
	using RenderContextFrames = std::vector<RenderContextFrame*>;

	obj<Device>         device            = {};

	RenderContextFrames renderFrames      = {};
	int32_t             frameIndex        = {};
	uint64_t            currentFrameID    = {};
	bool                dynamicFrameCount = {};
};

VERA_NAMESPACE_END