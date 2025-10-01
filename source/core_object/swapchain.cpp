#include "../../include/vera/core/swapchain.h"
#include "../impl/device_impl.h"
#include "../impl/framebuffer_impl.h"
#include "../impl/render_context_impl.h"
#include "../impl/semaphore_impl.h"
#include "../impl/swapchain_impl.h"
#include "../impl/texture_impl.h"
#include "../impl/window_impl.h"

#include "../../include/vera/core/context.h"
#include "../../include/vera/core/device.h"
#include "../../include/vera/core/device_memory.h"
#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/framebuffer.h"
#include "../../include/vera/core/semaphore.h"
#include "../../include/vera/core/render_context.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/os/window.h"

#define SWAPCHAIN_COLOR_SPACE \
	vk::ColorSpaceKHR::eSrgbNonlinear

#define SWAPCHAIN_IMAGE_USAGE             \
	ImageUsageFlagBits::ColorAttachment | \
	ImageUsageFlagBits::TransferSrc |     \
	ImageUsageFlagBits::TransferDst

VERA_NAMESPACE_BEGIN

class SwapchainFactory : protected CoreObject {
public:
	static obj<FrameBuffer> createFrameBuffer(SwapchainImpl& swapchain_impl)
	{
		return createNewObject<FrameBuffer>();
	}

	static obj<Texture> createTexture(SwapchainImpl& swapchain_impl)
	{
		auto  obj         = createNewObject<Texture>();
		auto& impl        = getImpl(obj);
		auto& device_impl = getImpl(swapchain_impl.device);

		impl.device      = swapchain_impl.device;
		impl.imageAspect = vk::ImageAspectFlagBits::eColor;
		impl.imageLayout = vk::ImageLayout::eUndefined;
		impl.imageFormat = swapchain_impl.imageFormat;

		return obj;
	}
};

static const char* get_glfw_error()
{
	const char* err_str = nullptr;
	glfwGetError(&err_str);
	return err_str;
}

static bool check_swapchain_present_mode(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, PresentMode mode)
{
	auto modes = physical_device.getSurfacePresentModesKHR(surface);
	auto iter  = std::find(VERA_SPAN(modes), to_vk_present_mode(mode));

	return iter != modes.cend();
}

static bool check_swapchain_format(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, Format format)
{
	auto formats        = physical_device.getSurfaceFormatsKHR(surface);
	auto surface_format = vk::SurfaceFormatKHR{ to_vk_format(format), SWAPCHAIN_COLOR_SPACE };
	auto iter           = std::find(VERA_SPAN(formats), surface_format);

	return iter != formats.cend();
}

static void recreate_swapchain_sync(SwapchainImpl& impl)
{
	impl.syncs.clear();

	while (impl.syncs.size() < impl.imageCount) {
		auto& sync = impl.syncs.emplace_back();
		sync.waitSemaphore = Semaphore::create(impl.device);
		sync.imageIndex    = -1;
	}
}

static void clear_swapchain_framebuffers(SwapchainImpl& impl)
{
	for (auto& framebuffer : impl.framebuffers) {
		auto& framebuffer_impl = CoreObject::getImpl(framebuffer);
		auto& texture_impl     = CoreObject::getImpl(framebuffer_impl.colorAttachment);

		texture_impl.image = nullptr;
	}

	impl.framebuffers.clear();
}

static void recreate_swapchain(DeviceImpl& device_impl, SwapchainImpl& impl)
{
	auto old_swapchain = impl.swapchain;
	auto capabilities  = device_impl.physicalDevice.getSurfaceCapabilitiesKHR(impl.surface);

	if (capabilities.maxImageCount == 0 || capabilities.maxImageCount < impl.imageCount)
		throw Exception("too many swapchain image count");
	if (!check_swapchain_present_mode(device_impl.physicalDevice, impl.surface, impl.presentMode))
		throw Exception("unsupported present mode");

	impl.width  = capabilities.currentExtent.width;
	impl.height = capabilities.currentExtent.height;

	device_impl.device.waitIdle();

	recreate_swapchain_sync(impl);

	vk::SwapchainCreateInfoKHR swapchain_info;
	swapchain_info.surface          = impl.surface;
	swapchain_info.minImageCount    = impl.imageCount;
	swapchain_info.imageFormat      = to_vk_format(impl.imageFormat);
	swapchain_info.imageColorSpace  = SWAPCHAIN_COLOR_SPACE;
	swapchain_info.imageExtent      = vk::Extent2D(impl.width, impl.height);
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage       = to_vk_image_usage_flags(SWAPCHAIN_IMAGE_USAGE);
	swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
	swapchain_info.preTransform     = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	swapchain_info.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchain_info.presentMode      = to_vk_present_mode(impl.presentMode);
	swapchain_info.clipped          = false;
	swapchain_info.oldSwapchain     = old_swapchain;

	impl.swapchain          = device_impl.device.createSwapchainKHR(swapchain_info);
	impl.acquiredImageIndex = -1;
	impl.syncIndex          = 0;

	// TODO: try without dynamic allocation
	clear_swapchain_framebuffers(impl);
	
	for (auto& swapchain_image : device_impl.device.getSwapchainImagesKHR(impl.swapchain)) {
		auto  framebuffer      = SwapchainFactory::createFrameBuffer(impl);
		auto  texture          = SwapchainFactory::createTexture(impl);
		auto& framebuffer_impl = CoreObject::getImpl(framebuffer);
		auto& texture_impl     = CoreObject::getImpl(texture);

		texture_impl.frameBuffer = framebuffer;
		texture_impl.image       = swapchain_image;
		texture_impl.imageUsage  = SWAPCHAIN_IMAGE_USAGE | ImageUsageFlagBits::FrameBuffer;
		texture_impl.width       = impl.width;
		texture_impl.height      = impl.height;
		texture_impl.depth       = 1;

		texture->getTextureView(); // create image view

		framebuffer_impl.device          = impl.device;
		framebuffer_impl.colorAttachment = std::move(texture);
		framebuffer_impl.width           = impl.width;
		framebuffer_impl.height          = impl.height;
		framebuffer_impl.format          = impl.imageFormat;

		impl.framebuffers.push_back(std::move(framebuffer));
	}

	device_impl.device.destroy(old_swapchain);
}

obj<Swapchain> Swapchain::create(obj<Device> device, os::Window& window, const SwapchainCreateInfo& info)
{
	VkSurfaceKHR surface;
	
	auto  obj              = createNewObject<Swapchain>();
	auto& impl             = getImpl(obj);
	auto& device_impl      = getImpl(device);
	auto& window_impl      = *window.m_impl;
	auto  vk_instance      = get_vk_instance(device_impl.context);
	auto  framebuffer_size = window.FramebufferSize.get();
	auto  format           = info.colorFormat == Format::Unknown ? device_impl.colorFormat : info.colorFormat;

	if (window_impl.swapchain)
		throw Exception("Window already bound to another swapchain");
	if (!glfwVulkanSupported())
		throw Exception("Vulkan is not supported by glfw");
	if (!glfwGetPhysicalDevicePresentationSupport(
	 	get_vk_instance(device_impl.context), device_impl.physicalDevice, device_impl.graphicsQueueFamilyIndex))
	 	throw Exception("Presentation is not supported by current physical device");
	if (glfwCreateWindowSurface(vk_instance, window_impl.window, nullptr, &surface) != VK_SUCCESS)
		throw Exception(get_glfw_error());
	if (!check_swapchain_format(device_impl.physicalDevice, surface, format))
		throw Exception("unsupported swapchain format");

	window_impl.swapchain = obj;

	impl.window             = &window;
	impl.device             = std::move(device);
	impl.surface            = surface;
	impl.imageFormat        = format;
	impl.presentMode        = info.presentMode;
	impl.imageCount         = info.imageCount;
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

	clear_swapchain_framebuffers(impl);

	device_impl.device.destroy(impl.swapchain);

	destroyObjectImpl(this);
}

obj<Device> Swapchain::getDevice()
{
	return getImpl(this).device;
}

ref<FrameBuffer> Swapchain::acquireNextImage()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
	auto& sync        = impl.syncs[impl.syncIndex];
	auto  trial       = 0u;

	if (sync.imageIndex != -1) {
		auto& framebuffer_impl = getImpl(impl.framebuffers[sync.imageIndex]);
		
		framebuffer_impl.frameSync.waitForRenderComplete();
	}

	while (trial++ < 10) {
		auto result = device_impl.device.acquireNextImageKHR(
			impl.swapchain,
			UINT64_MAX,
			get_vk_semaphore(sync.waitSemaphore));

		if (result.result == vk::Result::eSuccess) {
			auto  framebuffer      = impl.framebuffers[result.value];
			auto& framebuffer_impl = getImpl(framebuffer);
			auto& texture_impl     = getImpl(framebuffer_impl.colorAttachment);

			impl.acquiredImageIndex        = result.value;
			sync.imageIndex                = result.value;
			framebuffer_impl.waitSemaphore = sync.waitSemaphore;
			texture_impl.imageLayout       = vk::ImageLayout::eUndefined;

			return framebuffer;
		} else if (result.result == vk::Result::eSuboptimalKHR || result.result == vk::Result::eNotReady) {
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
	auto& texture_impl = getImpl(impl.framebuffers[impl.acquiredImageIndex]);
	auto  semaphore    = texture_impl.frameSync.getRenderCompleteSemaphore();

	if (!semaphore) {
		device_impl.device.waitIdle();
		throw Exception("cannot find frame for current image");
	}

	vk::PresentInfoKHR present_info;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &get_vk_semaphore(semaphore);
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = &impl.swapchain;
	present_info.pImageIndices      = reinterpret_cast<uint32_t*>(&impl.acquiredImageIndex);

	auto result = device_impl.graphicsQueue.presentKHR(&present_info);

	impl.syncIndex = (impl.syncIndex + 1) % impl.imageCount;
}

ref<FrameBuffer> Swapchain::getCurrentFrameBuffer()
{
	auto& impl = getImpl(this);

	return impl.framebuffers[impl.acquiredImageIndex];
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