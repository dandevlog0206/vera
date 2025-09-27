#pragma once

#include "object_impl.h"

#include "../../include/vera/core/swapchain.h"

VERA_NAMESPACE_BEGIN

struct SwapchainSync
{
	obj<Semaphore> waitSemaphore;
	obj<Fence>     fence;
};

struct SwapchainFrame
{
	obj<Texture> texture;
};

struct SwapchainImpl
{
	os::Window* window;
	obj<Device>                 device;
	obj<RenderContext>          renderContext;

	vk::SwapchainKHR            swapchain;
	vk::SurfaceKHR              surface;

	std::vector<SwapchainFrame> frames;
	std::vector<SwapchainSync>  syncs;
	Format                      imageFormat;
	PresentMode                 presentMode;
	uint32_t                    imageCount;
	uint32_t                    width;
	uint32_t                    height;
	int32_t                     acquiredImageIndex;
	int32_t                     syncIndex;
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