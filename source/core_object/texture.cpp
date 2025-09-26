#include "../../include/vera/core/texture.h"
#include "../impl/device_impl.h"
#include "../impl/device_memory_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/device_memory.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/render_command.h"
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

static void allocate_device_memory(
	DeviceMemoryImpl&   impl,
	ref<Device>         device,
	vk::Image           image,
	MemoryPropertyFlags flags
) {
	auto& device_impl    = CoreObject::getImpl(device);
	auto  req            = device_impl.device.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo info;
	info.allocationSize  = req.size;
	info.memoryTypeIndex = find_memory_type_idx(device_impl, flags, req.memoryTypeBits);

	impl.device        = std::move(device);
	impl.memory        = device_impl.device.allocateMemory(info);
	impl.propertyFlags = flags;
	impl.allocated     = info.allocationSize;
	impl.typeIndex     = info.memoryTypeIndex;
	impl.mapPtr        = nullptr;
}

vk::Image get_vk_image(const ref<Texture>& texture)
{
	return CoreObject::getImpl(texture).image;
}

ref<Texture> Texture::create(ref<Device> device, const TextureCreateInfo& info)
{
	auto  obj         = createNewObject<Texture>();
	auto  memory_obj  = createNewObject<DeviceMemory>();
	auto& impl        = getImpl(obj);
	auto& memory_impl = getImpl(memory_obj);
	auto  vk_device   = get_vk_device(device);

	impl.device           = std::move(device);
	impl.deviceMemory     = std::move(memory_obj);
	impl.imageUsage       = 
		vk::ImageUsageFlagBits::eColorAttachment |
		vk::ImageUsageFlagBits::eSampled |
		vk::ImageUsageFlagBits::eTransferDst;
	impl.imageAspect      = vk::ImageAspectFlagBits::eColor;
	impl.imageLayout      = vk::ImageLayout::eUndefined;
	impl.imageFormat      = info.format;
	impl.width            = info.width;
	impl.height           = info.height;
	impl.depth            = info.depth;
	impl.isSwapchainImage = false;

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
	image_info.usage         = impl.imageUsage;
	image_info.sharingMode   = vk::SharingMode::eExclusive;

	impl.image = vk_device.createImage(image_info);

	allocate_device_memory(
		memory_impl,
		impl.device,
		impl.image,
		MemoryPropertyFlagBits::DeviceLocal);

	auto& binding = memory_impl.resourceBind.emplace_back();
	binding.resourceType = MemoryResourceType::Texture;
	binding.size         = memory_impl.allocated;
	binding.offset       = 0;
	binding.resourcePtr  = obj.get();

	impl.size = memory_impl.allocated;

	vk_device.bindImageMemory(impl.image, memory_impl.memory, 0);

	return obj;
}

Texture::~Texture()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	// TODO: change to assert
	if (impl.textureView.count() != 1)
		throw Exception("default texture view must not be owned by others");

	impl.textureView.reset();

	// TODO: unbind device memory

	vk_device.destroy(impl.image);

	destroyObjectImpl(this);
}

void Texture::upload(const Image& image)
{
	auto& impl           = getImpl(this);
	auto  image_size     = image.size();
	auto  staging_buffer = Buffer::createStaging(impl.device, image_size);
	auto  command_buffer = RenderCommand::create(impl.device);
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
		vr::ImageLayout::Undefined,
		vr::ImageLayout::TransferDstOptimal);

	command_buffer->copyBufferToTexture(
		ref<Texture>(this),
		staging_buffer,
		0,
		0,
		0,
		int3(),
		extent3d(impl.width, impl.height, impl.depth));

	command_buffer->transitionImageLayout(
		ref<Texture>(this),
		vr::PipelineStageFlagBits::Transfer,
		vr::PipelineStageFlagBits::FragmentShader,
		vr::AccessFlagBits::TransferWrite,
		vr::AccessFlagBits::ShaderRead,
		vr::ImageLayout::TransferDstOptimal,
		vr::ImageLayout::ShaderReadOnlyOptimal);
	command_buffer->end();

	impl.device->submitCommand(command_buffer);
	impl.device->waitIdle();
}

ref<DeviceMemory> Texture::getDeviceMemory()
{
	return getImpl(this).deviceMemory;
}

ref<TextureView> Texture::getTextureView()
{
	auto& impl = getImpl(this);
	
	if (!impl.textureView) {
		auto  obj       = createNewObject<TextureView>();
		auto& view_impl = getImpl(obj);
		auto  vk_device = get_vk_device(impl.device);

		vk::ImageViewCreateInfo view_info;
		view_info.image                           = impl.image;
		view_info.viewType                        = vk::ImageViewType::e2D;
		view_info.format                          = to_vk_format(impl.imageFormat);
		view_info.components.r                    = vk::ComponentSwizzle::eIdentity;
		view_info.components.g                    = vk::ComponentSwizzle::eIdentity;
		view_info.components.b                    = vk::ComponentSwizzle::eIdentity;
		view_info.components.a                    = vk::ComponentSwizzle::eIdentity;
		view_info.subresourceRange.aspectMask     = impl.imageAspect;
		view_info.subresourceRange.baseMipLevel   = 0;
		view_info.subresourceRange.levelCount     = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount     = 1;

		view_impl.device    = impl.device;
		view_impl.texture   = ref<Texture>(this);
		view_impl.imageView = vk_device.createImageView(view_info);
		view_impl.offsetX   = 0;
		view_impl.offsetY   = 0;
		view_impl.width     = impl.width;
		view_impl.height    = impl.height;
		view_impl.depth     = impl.depth;

		return impl.textureView = obj;
	}

	return impl.textureView;
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

VERA_NAMESPACE_END