#pragma once

#include "render_context_impl.h"
#include "../../include/vera/core/command_buffer_sync.h"

VERA_NAMESPACE_BEGIN

struct FrameBufferImpl
{
	obj<Device>       device;
	obj<Texture>      colorAttachment;
	obj<Texture>      depthAttachment;
	obj<Texture>      stencilAttachment;
	obj<Semaphore>    waitSemaphore;

	uint32_t          width             = {};
	uint32_t          height            = {};
	Format            format            = Format::Unknown;
	DepthFormat       depthFormat       = DepthFormat::Unknown;
	StencilFormat     stencilFormat     = StencilFormat::Unknown;
	CommandBufferSync commandBufferSync = {};
};

VERA_NAMESPACE_END