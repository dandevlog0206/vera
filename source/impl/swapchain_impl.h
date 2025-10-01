#pragma once

#include "render_context_impl.h"

VERA_NAMESPACE_BEGIN

struct SwapchainSync
{
	obj<Semaphore> waitSemaphore;
	int32_t        imageIndex;
};

struct SwapchainImpl
{
	os::Window* window;
	obj<Device>                   device;

	vk::SwapchainKHR              swapchain;
	vk::SurfaceKHR                surface;

	std::vector<obj<FrameBuffer>> framebuffers;
	std::vector<SwapchainSync>    syncs;
	Format                        imageFormat;
	PresentMode                   presentMode;
	uint32_t                      imageCount;
	uint32_t                      width;
	uint32_t                      height;
	int32_t                       acquiredImageIndex;
	int32_t                       syncIndex;
};

static vk::PresentModeKHR to_vk_present_mode(PresentMode mode)
{
	switch (mode) {
	case PresentMode::Fifo:      return vk::PresentModeKHR::eFifo;
	case PresentMode::Immediate: return vk::PresentModeKHR::eImmediate;
	case PresentMode::Mailbox:   return vk::PresentModeKHR::eMailbox;
	}

	VERA_ASSERT_MSG(false, "invalid present mode");
	return {};
}

VERA_NAMESPACE_END