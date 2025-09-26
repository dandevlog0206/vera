#include "../../include/vera/core/swapchain.h"
#include "../impl/device_impl.h"
#include "../impl/render_context_impl.h"
#include "../impl/semaphore_impl.h"
#include "../impl/swapchain_impl.h"
#include "../impl/texture_impl.h"
#include "../impl/window_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/device_memory.h"
#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/semaphore.h"
#include "../../include/vera/core/render_context.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/os/window.h"

#define SWAPCHAIN_IMAGE_USAGE                  \
	vk::ImageUsageFlagBits::eColorAttachment | \
	vk::ImageUsageFlagBits::eTransferSrc |     \
	vk::ImageUsageFlagBits::eTransferDst

VERA_NAMESPACE_BEGIN

class SwapchainTextureFactory : protected CoreObject {
public:
	static ref<Texture> create(SwapchainImpl& swapchain_impl)
	{
		auto  obj            = createNewObject<Texture>();
		auto& impl           = getImpl(obj);
		auto& device_impl    = getImpl(swapchain_impl.device);

		impl.device           = swapchain_impl.device;
		impl.imageUsage       = SWAPCHAIN_IMAGE_USAGE;
		impl.imageAspect      = vk::ImageAspectFlagBits::eColor;
		impl.imageLayout      = vk::ImageLayout::eUndefined;
		impl.imageFormat      = swapchain_impl.imageFormat;
		impl.isSwapchainImage = true;

		return obj;
	}
};

static const char* get_glfw_error()
{
	const char* err_str = nullptr;
	glfwGetError(&err_str);
	return err_str;
}

static void prepare_swapchain_sync(SwapchainImpl& impl)
{
	for (auto& sync : impl.syncs)
		sync.fence = {};

	while (impl.syncs.size() < impl.imageCount) {
		auto& sync = impl.syncs.emplace_back();

		sync.waitSemaphore = Semaphore::create(impl.device);
		sync.fence         = {};
	}

	impl.syncs.resize(impl.imageCount);
}

static void clear_swapchain_frames(SwapchainImpl& impl)
{
	for (auto& frame : impl.frames) {
		auto& texture_impl = CoreObject::getImpl(frame.texture);

		texture_impl.image = nullptr;
	}

	impl.frames.clear();
}

static void recreate_swapchain(DeviceImpl& device_impl, SwapchainImpl& impl)
{
	auto old_swapchain = impl.swapchain;

	device_impl.device.waitIdle();

	prepare_swapchain_sync(impl);

	vk::SwapchainCreateInfoKHR swapchain_info;
	swapchain_info.surface          = impl.surface;
	swapchain_info.minImageCount    = impl.imageCount;
	swapchain_info.imageFormat      = to_vk_format(impl.imageFormat);
	swapchain_info.imageColorSpace  = vk::ColorSpaceKHR::eSrgbNonlinear;
	swapchain_info.imageExtent      = vk::Extent2D(impl.width, impl.height);
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage       = SWAPCHAIN_IMAGE_USAGE;
	swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
	swapchain_info.preTransform     = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	swapchain_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchain_info.presentMode      = to_vk_present_mode(impl.presentMode);
	swapchain_info.clipped          = false;
	swapchain_info.oldSwapchain     = old_swapchain;

	impl.swapchain          = device_impl.device.createSwapchainKHR(swapchain_info);
	impl.acquiredImageIndex = -1;
	impl.syncIndex          = 0;

	clear_swapchain_frames(impl);
	
	for (auto& swapchain_image : device_impl.device.getSwapchainImagesKHR(impl.swapchain)) {
		auto  texture      = SwapchainTextureFactory::create(impl);
		auto& texture_impl = CoreObject::getImpl(texture);
		auto& frame        = impl.frames.emplace_back();

		texture_impl.image       = swapchain_image;
		texture_impl.imageUsage  = SWAPCHAIN_IMAGE_USAGE;
		texture_impl.width       = impl.width;
		texture_impl.height      = impl.height;
		texture_impl.depth       = 1;

		texture->getTextureView(); // create image view

		frame.texture = texture;
	}

	device_impl.device.destroy(old_swapchain);
}

ref<Swapchain> Swapchain::create(ref<RenderContext> render_ctx, os::Window& window, const SwapchainCreateInfo& info)
{
	VkSurfaceKHR surface;
	
	auto  obj              = createNewObject<Swapchain>();
	auto& impl             = getImpl(obj);
	auto& ctx_impl         = getImpl(render_ctx);
	auto& device_impl      = getImpl(ctx_impl.device);
	auto& window_impl      = *window.m_impl;
	auto  vk_instance      = get_vk_instance(device_impl.context);
	auto  framebuffer_size = window.FramebufferSize.get();

	if (window_impl.swapchain)
		throw Exception("Window already bound to another swapchain");
	if (!glfwVulkanSupported())
		throw Exception("Vulkan is not supported by glfw");
	// if (!glfwGetPhysicalDevicePresentationSupport(
	// 	context_impl.instance, device_impl.physicalDevice, device_impl.graphicsQueueFamilyIndex))
	// 	throw Exception("Presentation is not supported by current physical device"); TODO: ????
	if (glfwCreateWindowSurface(vk_instance, window_impl.window, nullptr, &surface) != VK_SUCCESS)
		throw Exception(get_glfw_error());

	window_impl.swapchain = obj;

	impl.window             = &window;
	impl.device             = ctx_impl.device;
	impl.renderContext      = std::move(render_ctx);
	impl.surface            = surface;
	impl.imageFormat        = info.colorFormat == Format::Unknown ? device_impl.colorFormat : info.colorFormat;
	impl.presentMode        = info.presentMode;
	impl.imageCount         = info.imageCount;
	impl.width              = info.width == 0 ? framebuffer_size.width : info.width;
	impl.height             = info.height == 0 ? framebuffer_size.height : info.height;
	impl.acquiredImageIndex = -1;
	impl.syncIndex          = 0;

	impl.syncs.reserve(impl.imageCount);

	recreate_swapchain(device_impl, impl);

	return obj;
}

Swapchain::~Swapchain()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	clear_swapchain_frames(impl);

	device_impl.device.destroy(impl.swapchain);

	destroyObjectImpl(this);
}

ref<Texture> Swapchain::acquireNextImage()
{
	auto& impl         = getImpl(this);
	auto& device_impl  = getImpl(impl.device);
	auto& ctx_impl     = getImpl(impl.renderContext);
	auto& render_frame = ctx_impl.renderFrames[ctx_impl.frameIndex];
	auto& sync         = impl.syncs[impl.syncIndex];
	auto  trial        = 0u;

	if (sync.fence)
		sync.fence->wait();
	
	while (trial++ < 10) {
		auto result = device_impl.device.acquireNextImageKHR(
			impl.swapchain,
			UINT64_MAX,
			get_vk_semaphore(sync.waitSemaphore));

		if (result.result == vk::Result::eSuccess) {
			auto& texture      = impl.frames[result.value].texture;
			auto& texture_impl = getImpl(texture);

			impl.acquiredImageIndex         = result.value;
			render_frame.imageWaitSemaphore = sync.waitSemaphore;
			texture_impl.imageLayout        = vk::ImageLayout::eUndefined;

			return texture;
		} else if (result.result == vk::Result::eSuboptimalKHR || result.result == vk::Result::eNotReady) {

			auto new_extent = impl.window->FramebufferSize.get();

			impl.width  = new_extent.width;
			impl.height = new_extent.height;

			recreate_swapchain(device_impl, impl);
		} else {
			throw Exception("failed to acquire next swapchain image");
		}
	}

	throw Exception("failed to recreate swapchain image");
}

void Swapchain::recreate()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	recreate_swapchain(device_impl, impl);
}

void Swapchain::present()
{
	auto& impl         = getImpl(this);
	auto& device_impl  = getImpl(impl.device);
	auto& context_impl = getImpl(impl.renderContext);
	auto& render_frame = context_impl.renderFrames[context_impl.lastFrameIndex];
	auto& frame        = impl.frames[impl.acquiredImageIndex];

	impl.syncs[impl.syncIndex].fence = render_frame.fence;

	vk::PresentInfoKHR present_info;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &get_vk_semaphore(render_frame.renderCompleteSemaphore);
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = &impl.swapchain;
	present_info.pImageIndices      = reinterpret_cast<uint32_t*>(&impl.acquiredImageIndex);

	auto result = device_impl.graphicsQueue.presentKHR(present_info);

	impl.syncIndex = (impl.syncIndex + 1) % impl.imageCount;
}

ref<Texture> Swapchain::getCurrentTexture()
{
	auto& impl = getImpl(this);

	return impl.frames[impl.acquiredImageIndex].texture;
}

extent2d Swapchain::getFrameBufferExtent() const
{
	auto& impl = getImpl(this);

	return { impl.width, impl.height };
}

void Swapchain::setPresentMode(PresentMode mode)
{
	getImpl(this).presentMode = mode;
	recreate();
}

bool Swapchain::isOccluded() const
{
	auto& impl = getImpl(this);

	return impl.window->Minimized;
}

VERA_NAMESPACE_END