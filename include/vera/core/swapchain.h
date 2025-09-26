#pragma once

#include "core_object.h"
#include "../graphics/format.h"
#include "../util/extent.h"

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

class Window;

VERA_OS_NAMESPACE_END

class RenderContext;
class Texture;

enum class PresentMode
{
	Fifo,
	Immediate,
	Mailbox
};

struct SwapchainCreateInfo
{
	uint32_t    width       = 0;
	uint32_t    height      = 0;
	uint32_t    imageCount  = 3;
	Format      colorFormat = {};
	PresentMode presentMode = PresentMode::Fifo;
};

class Swapchain : protected CoreObject
{
	VERA_CORE_OBJECT(Swapchain)
public:
	static ref<Swapchain> create(ref<RenderContext> render_ctx, os::Window& window, const SwapchainCreateInfo& info = {});
	~Swapchain();

	ref<Texture> acquireNextImage();
	void recreate();
	void present();

	ref<Texture> getCurrentTexture();

	extent2d getFrameBufferExtent() const;

	void setPresentMode(PresentMode mode);
	bool isOccluded() const;
};

VERA_NAMESPACE_END