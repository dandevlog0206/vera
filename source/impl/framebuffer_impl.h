#pragma once

#include "render_context_impl.h"

VERA_NAMESPACE_BEGIN

struct FrameBufferImpl
{
	obj<Device>    device;
	obj<Texture>   colorAttachment;
	obj<Texture>   depthAttachment;
	obj<Texture>   stencilAttachment;
	obj<Semaphore> waitSemaphore;

	uint32_t       width         = {};
	uint32_t       height        = {};
	Format         format        = Format::Unknown;
	DepthFormat    depthFormat   = DepthFormat::Unknown;
	StencilFormat  stencilFormat = StencilFormat::Unknown;
	FrameSync      frameSync     = {};
};

VERA_NAMESPACE_END