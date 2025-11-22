#pragma once

#include "device.h"
#include "../util/extent.h"

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

class Window;

VERA_OS_NAMESPACE_END

class FrameBuffer;

struct SwapchainCreateInfo
{
	uint32_t    imageCount  = 3;
	Format      colorFormat = {};
	PresentMode presentMode = PresentMode::Fifo;
};

class Swapchain : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Swapchain)
public:
	static obj<Swapchain> create(obj<Device> device, os::Window& window, const SwapchainCreateInfo& info = {});
	~Swapchain() VERA_NOEXCEPT override;

	obj<Device> getDevice();

	ref<FrameBuffer> acquireNextImage();
	void recreate();
	void present();

	ref<FrameBuffer> getCurrentFrameBuffer();

	extent2d getFrameBufferExtent() const;

	void setPresentMode(PresentMode mode);
	bool isOccluded() const;
};

VERA_NAMESPACE_END