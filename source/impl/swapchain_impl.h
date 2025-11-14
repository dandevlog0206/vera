#pragma once

#include "render_context_impl.h"

#define MAX_SWAPCHAIN_IMAGE_COUNT 8

VERA_NAMESPACE_BEGIN

struct SwapchainSync
{
	obj<Semaphore> waitSemaphore;
	int32_t        imageIndex;
};

class SwapchainImpl
{
public:
	os::Window*                   window             = {};
	obj<Device>                   device             = {};

	vk::SwapchainKHR              swapchain          = {};
	vk::SurfaceKHR                surface            = {};

	std::vector<obj<FrameBuffer>> framebuffers       = {};
	std::vector<SwapchainSync>    syncs              = {};
	Format                        imageFormat        = {};
	PresentMode                   presentMode        = {};
	uint32_t                      imageCount         = {};
	uint32_t                      width              = {};
	uint32_t                      height             = {};
	int32_t                       acquiredImageIndex = {};
	int32_t                       syncIndex          = {};
};

VERA_NAMESPACE_END