#pragma once

#include "render_context_impl.h"
#include "../../include/vera/core/command_sync.h"

VERA_NAMESPACE_BEGIN

class FrameBufferImpl
{
public:
	obj<Device>       device            = {};
	obj<Texture>      colorAttachment   = {};
	obj<Texture>      depthAttachment   = {};
	obj<Texture>      stencilAttachment = {};
	obj<Semaphore>    waitSemaphore     = {};

	uint32_t          width             = {};
	uint32_t          height            = {};
	Format            format            = Format::Unknown;
	DepthFormat       depthFormat       = DepthFormat::Unknown;
	StencilFormat     stencilFormat     = StencilFormat::Unknown;
	CommandSync       commandSync       = {};
};

VERA_NAMESPACE_END