#include "../../include/vera/core/texture_view.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/texture.h"

VERA_NAMESPACE_BEGIN

const vk::ImageView& get_vk_image_view(const_ref<TextureView> texture_view) VERA_NOEXCEPT
{
	return CoreObject::getImpl(texture_view).imageView;
}

vk::ImageView& get_vk_image_view(ref<TextureView> texture_view) VERA_NOEXCEPT
{
	return CoreObject::getImpl(texture_view).imageView;
}

obj<TextureView> TextureView::create(obj<Texture> texture, const TextureViewCreateInfo& info)
{
	auto  obj          = createNewCoreObject<TextureView>();
	auto& impl         = getImpl(obj);
	auto& texture_impl = getImpl(texture);
	auto  vk_device    = get_vk_device(texture_impl.device);

	vk::ImageViewCreateInfo view_info;
	view_info.image                           = texture_impl.image;
	view_info.viewType                        = to_vk_image_view_type(info.type);
	view_info.format                          = to_vk_format(info.format);
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
	impl.imageView      = vk_device.createImageView(view_info);
	impl.width          = texture_impl.width;
	impl.height         = texture_impl.height;
	impl.depth          = texture_impl.depth;
	impl.type           = info.type;
	impl.format         = info.format;
	impl.mapping        = info.mapping;
	impl.aspectFlags    = info.aspectFlags;
	impl.baseMipLevel   = info.baseMipLevel;
	impl.levelCount     = info.levelCount;
	impl.baseArrayLayer = info.baseArrayLayer;
	impl.layerCount     = info.layerCount;

	return obj;
}

TextureView::~TextureView()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.destroy(impl.imageView);

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