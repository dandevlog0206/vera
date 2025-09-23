#include "../../include/vera/core/texture.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/device_memory.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

vk::Image get_vk_image(const ref<Texture>& texture)
{
	return CoreObject::getImpl(texture).image;
}

ref<Texture> Texture::create(ref<Device> device, const TextureCreateInfo& info)
{
	auto  obj       = createNewObject<Texture>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(impl.device);

	impl.device = std::move(device);

	return obj;
}

Texture::~Texture()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	VERA_ASSERT(impl.textureView.count() == 1);

	impl.textureView.reset();

	vk_device.destroy(impl.image);

	destroyObjectImpl(this);
}

ref<DeviceMemory> Texture::getDeviceMemory()
{
	// TODO: fix
	return {}; // getImpl(this).deviceMemory;
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