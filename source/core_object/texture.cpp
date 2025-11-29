#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"
#include "../impl/device_impl.h"
#include "../impl/device_memory_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/device_memory.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/graphics/image.h"

VERA_NAMESPACE_BEGIN

static vk::ImageType get_image_type(const TextureCreateInfo& info)
{
	if (info.height == 1 && info.depth == 1)
		return vk::ImageType::e1D;
	if (info.depth == 1)
		return vk::ImageType::e2D;
	return vk::ImageType::e3D;
}

static TextureUsageFlags get_image_usage_flags(Format format)
{
	switch (format) {
	case Format::D16Unorm:
	case Format::X8D24Unorm:
	case Format::D32Float:
	case Format::S8Uint:
	case Format::D16UnormS8Uint:
	case Format::D24UnormS8Uint:
	case Format::D32FloatS8Uint:
		return
			TextureUsageFlagBits::DepthStencilAttachment;
	default:
		return
			TextureUsageFlagBits::ColorAttachment |
			TextureUsageFlagBits::Sampled |
			TextureUsageFlagBits::TransferDst;
	}
}

static vk::SampleCountFlagBits get_sample_count(uint32_t count)
{
	switch (count) {
	case 1:  return vk::SampleCountFlagBits::e1;
	case 2:  return vk::SampleCountFlagBits::e2;
	case 4:  return vk::SampleCountFlagBits::e4;
	case 8:  return vk::SampleCountFlagBits::e8;
	case 16: return vk::SampleCountFlagBits::e16;
	case 32: return vk::SampleCountFlagBits::e32;
	case 64: return vk::SampleCountFlagBits::e64;
	}

	throw Exception("invalid sample count");
}

static uint32_t find_texture_bind_idx(DeviceMemoryImpl& impl, Texture* this_ptr)
{
	auto iter = std::find_if(VERA_SPAN(impl.resourceBind),
		[=](const auto& bind) {
			return bind.resourcePtr == this_ptr;
		});

	VERA_ASSERT(iter != impl.resourceBind.end());

	return static_cast<uint32_t>(iter - impl.resourceBind.cbegin());
}

static void allocate_device_memory(
	DeviceMemoryImpl&   impl,
	obj<Device>         device,
	vk::Image           image,
	MemoryPropertyFlags flags
) {
	auto& device_impl    = CoreObject::getImpl(device);
	auto  req            = device_impl.vkDevice.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo info;
	info.allocationSize  = req.size;
	info.memoryTypeIndex = device_impl.findMemoryTypeIndex(flags, req.memoryTypeBits);

	impl.device        = std::move(device);
	impl.vkMemory      = device_impl.vkDevice.allocateMemory(info);
	impl.propertyFlags = flags;
	impl.allocated     = info.allocationSize;
	impl.typeIndex     = info.memoryTypeIndex;
	impl.mapPtr        = nullptr;
}

const vk::Image& get_vk_image(cref<Texture> texture) VERA_NOEXCEPT
{
	return CoreObject::getImpl(texture).vkImage;
}

vk::Image& get_vk_image(ref<Texture> texture) VERA_NOEXCEPT
{
	return CoreObject::getImpl(texture).vkImage;
}

obj<Texture> Texture::createDepth(obj<Device> device, uint32_t width, uint32_t height, DepthFormat format)
{
	TextureCreateInfo info = {
		.type   = TextureType::Texture2D,
		.format = static_cast<Format>(format),
		.width  = width,
		.height = height,
	};

	auto  obj  = create(device, info);
	auto& impl = getImpl(obj);

	impl.textureUsage  = TextureUsageFlagBits::DepthStencilAttachment;
	impl.textureAspect = TextureAspectFlagBits::Depth;

	return obj;
}

obj<Texture> Texture::createStencil(obj<Device> device, uint32_t width, uint32_t height, StencilFormat format)
{
	TextureCreateInfo info = {
		.type   = TextureType::Texture2D,
		.format = static_cast<Format>(format),
		.width  = width,
		.height = height,
	};

	auto  obj  = create(device, info);
	auto& impl = getImpl(obj);

	impl.textureUsage  = TextureUsageFlagBits::DepthStencilAttachment;
	impl.textureAspect = TextureAspectFlagBits::Stencil;

	return obj;
}

obj<Texture> Texture::create(obj<Device> device, const Image & image)
{
	auto obj = create(device, TextureCreateInfo{
		.type       = TextureType::Texture2D,
		.format     = image.format(),
		.width      = image.width(),
		.height     = image.height(),
	});

	obj->upload(image);

	return obj;
}

obj<Texture> Texture::create(obj<Device> device, const TextureCreateInfo& info)
{
	auto  obj         = createNewCoreObject<Texture>();
	auto  memory_obj  = createNewCoreObject<DeviceMemory>();
	auto& impl        = getImpl(obj);
	auto& memory_impl = getImpl(memory_obj);
	auto  vk_device   = get_vk_device(device);

	impl.device        = std::move(device);
	impl.deviceMemory  = std::move(memory_obj);
	impl.textureFormat = info.format;
	impl.textureUsage  = info.usage ? info.usage : get_image_usage_flags(info.format);
	impl.textureAspect = TextureAspectFlagBits::Color;
	impl.textureLayout = TextureLayout::Undefined;
	impl.width         = info.width;
	impl.height        = info.height;
	impl.depth         = info.depth;

	vk::ImageCreateInfo image_info;
	image_info.imageType     = get_image_type(info);
	image_info.format        = to_vk_format(info.format);
	image_info.extent.width  = info.width;
	image_info.extent.height = info.height;
	image_info.extent.depth  = info.depth;
	image_info.mipLevels     = info.mipLevels;
	image_info.arrayLayers   = info.arraySize;
	image_info.samples       = get_sample_count(info.sampleCount);
	image_info.tiling        = vk::ImageTiling::eOptimal;
	image_info.usage         = to_vk_image_usage_flags(impl.textureUsage);
	image_info.sharingMode   = vk::SharingMode::eExclusive;

	impl.vkImage = vk_device.createImage(image_info);

	allocate_device_memory(
		memory_impl,
		impl.device,
		impl.vkImage,
		MemoryPropertyFlagBits::DeviceLocal);

	auto& binding = memory_impl.resourceBind.emplace_back();
	binding.resourceType = MemoryResourceType::Texture;
	binding.size         = memory_impl.allocated;
	binding.offset       = 0;
	binding.resourcePtr  = obj.get();

	impl.size = memory_impl.allocated;

	vk_device.bindImageMemory(impl.vkImage, memory_impl.vkMemory, 0);

	return obj;
}

Texture::~Texture() VERA_NOEXCEPT
{
	auto& impl        = getImpl(this);
	auto& memory_impl = getImpl(impl.deviceMemory);
	auto  vk_device   = get_vk_device(impl.device);

	VERA_ASSERT_MSG(impl.textureView ? impl.textureView.count() == 1 : true,
					"default texture view must not be owned by others");

	impl.textureView.reset();

	auto idx = find_texture_bind_idx(memory_impl, this);
	std::swap(memory_impl.resourceBind[idx], memory_impl.resourceBind.back());
	memory_impl.resourceBind.pop_back();

	vk_device.destroy(impl.vkImage);

	destroyObjectImpl(this);
}

void Texture::upload(const Image& image)
{
	auto& impl           = getImpl(this);
	auto  image_size     = image.size();
	auto  staging_buffer = Buffer::createStaging(impl.device, image_size);
	auto  command_buffer = CommandBuffer::create(impl.device);
	auto  buffer_memory  = staging_buffer->getDeviceMemory();

	auto* map = buffer_memory->map();
	memcpy(map, image.data(), image_size);
	buffer_memory->flush();

	command_buffer->begin();
	command_buffer->transitionImageLayout(
		ref<Texture>(this),
		vr::PipelineStageFlagBits::TopOfPipe,
		vr::PipelineStageFlagBits::Transfer,
		vr::AccessFlagBits{},
		vr::AccessFlagBits::TransferWrite,
		vr::TextureLayout::Undefined,
		vr::TextureLayout::TransferDstOptimal);

	command_buffer->copyBufferToTexture(
		ref<Texture>(this),
		staging_buffer,
		0,
		0,
		0,
		uint3(),
		extent3d(impl.width, impl.height, impl.depth));

	command_buffer->transitionImageLayout(
		ref<Texture>(this),
		vr::PipelineStageFlagBits::Transfer,
		vr::PipelineStageFlagBits::FragmentShader,
		vr::AccessFlagBits::TransferWrite,
		vr::AccessFlagBits::ShaderRead,
		vr::TextureLayout::TransferDstOptimal,
		vr::TextureLayout::ShaderReadOnlyOptimal);
	command_buffer->end();

	command_buffer->submit().wait();
}

obj<Device> Texture::getDevice()
{
	return getImpl(this).device;
}

obj<DeviceMemory> Texture::getDeviceMemory()
{
	return getImpl(this).deviceMemory;
}

ref<TextureView> Texture::getTextureView()
{
	auto& impl = getImpl(this);
	
	if (!impl.textureView) {
		auto  obj       = createNewCoreObject<TextureView>();
		auto& view_impl = getImpl(obj);
		auto  vk_device = get_vk_device(impl.device);

		vk::ImageViewCreateInfo view_info;
		view_info.image                           = impl.vkImage;
		view_info.viewType                        = vk::ImageViewType::e2D;
		view_info.format                          = to_vk_format(impl.textureFormat);
		view_info.components.r                    = vk::ComponentSwizzle::eIdentity;
		view_info.components.g                    = vk::ComponentSwizzle::eIdentity;
		view_info.components.b                    = vk::ComponentSwizzle::eIdentity;
		view_info.components.a                    = vk::ComponentSwizzle::eIdentity;
		view_info.subresourceRange.aspectMask     = to_vk_image_aspect_flags(impl.textureAspect);
		view_info.subresourceRange.baseMipLevel   = 0;
		view_info.subresourceRange.levelCount     = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount     = 1;

		view_impl.device      = impl.device;
		view_impl.texture     = vr::obj<Texture>(this);
		view_impl.vkImageView = vk_device.createImageView(view_info);
		view_impl.width       = impl.width;
		view_impl.height      = impl.height;
		view_impl.depth       = impl.depth;

		return impl.textureView = obj;
	}

	return impl.textureView;
}

Format Texture::getFormat() const
{
	return getImpl(this).textureFormat;
}

TextureUsageFlags Texture::getUsageFlags() const
{
	return getImpl(this).textureUsage;
}

uint32_t Texture::width() const
{
	return getImpl(this).width;
}

uint32_t Texture::height() const
{
	return getImpl(this).height;
}

uint32_t Texture::depth() const
{
	return getImpl(this).depth;
}

extent3d Texture::extent() const
{
	auto& impl = getImpl(this);
	
	return { impl.width, impl.height, impl.depth };
}

const vk::ImageView& get_vk_image_view(cref<TextureView> texture_view) VERA_NOEXCEPT
{
	return CoreObject::getImpl(texture_view).vkImageView;
}

vk::ImageView& get_vk_image_view(ref<TextureView> texture_view) VERA_NOEXCEPT
{
	return CoreObject::getImpl(texture_view).vkImageView;
}

obj<TextureView> TextureView::create(obj<Texture> texture, const TextureViewCreateInfo& info)
{
	auto  obj          = createNewCoreObject<TextureView>();
	auto& impl         = getImpl(obj);
	auto& texture_impl = getImpl(texture);
	auto  vk_device    = get_vk_device(texture_impl.device);
	auto  format       = info.format != Format::Unknown ? info.format : texture_impl.textureFormat;

	vk::ImageViewCreateInfo view_info;
	view_info.image                           = texture_impl.vkImage;
	view_info.viewType                        = to_vk_image_view_type(info.type);
	view_info.format                          = to_vk_format(format);
	view_info.components.r                    = to_vk_component_swizzle(info.mapping.r);
	view_info.components.g                    = to_vk_component_swizzle(info.mapping.g);
	view_info.components.b                    = to_vk_component_swizzle(info.mapping.b);
	view_info.components.a                    = to_vk_component_swizzle(info.mapping.a);
	view_info.subresourceRange.aspectMask     = to_vk_image_aspect_flags(info.aspectFlags);
	view_info.subresourceRange.baseMipLevel   = info.baseMipLevel;
	view_info.subresourceRange.levelCount     = info.levelCount;
	view_info.subresourceRange.baseArrayLayer = info.baseArrayLayer;
	view_info.subresourceRange.layerCount     = info.layerCount;

	impl.device         = texture_impl.device;
	impl.texture        = std::move(texture);
	impl.vkImageView    = vk_device.createImageView(view_info);
	impl.width          = texture_impl.width;
	impl.height         = texture_impl.height;
	impl.depth          = texture_impl.depth;
	impl.type           = info.type;
	impl.format         = format;
	impl.mapping        = info.mapping;
	impl.aspectFlags    = info.aspectFlags;
	impl.baseMipLevel   = info.baseMipLevel;
	impl.levelCount     = info.levelCount;
	impl.baseArrayLayer = info.baseArrayLayer;
	impl.layerCount     = info.layerCount;

	return obj;
}

TextureView::~TextureView() VERA_NOEXCEPT
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.destroy(impl.vkImageView);

	destroyObjectImpl(this);
}

obj<Device> TextureView::getDevice()
{
	return getImpl(this).device;
}

obj<Texture> TextureView::getTexture()
{
	return getImpl(this).texture;
}

TextureViewType TextureView::getType() const
{
	return getImpl(this).type;
}

Format TextureView::getFormat() const
{
	return getImpl(this).format;
}

ComponentMapping TextureView::getComponentMapping() const
{
	return getImpl(this).mapping;
}

TextureSubresourceRange TextureView::getTextureSubresourceRange() const
{
	auto& impl = getImpl(this);

	return TextureSubresourceRange{
		.aspectFlags    = impl.aspectFlags,
		.baseMipLevel   = impl.baseMipLevel,
		.levelCount     = impl.levelCount,
		.baseArrayLayer = impl.baseArrayLayer,
		.layerCount     = impl.layerCount
	};
}

uint32_t TextureView::width() const
{
	return getImpl(this).width;
}

uint32_t TextureView::height() const
{
	return getImpl(this).height;
}

uint32_t TextureView::depth() const
{
	return getImpl(this).depth;
}

extent3d TextureView::extent() const
{
	auto& impl = getImpl(this);
	return { impl.width, impl.height, impl.depth };
}

VERA_NAMESPACE_END