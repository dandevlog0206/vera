#include "../../include/vera/core/texture_view.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"

VERA_NAMESPACE_BEGIN

vk::ImageView& get_vk_image_view(ref<TextureView> texture_view)
{
	return CoreObject::getImpl(texture_view).imageView;
}

obj<TextureView> TextureView::create(obj<Texture> texture, const TextureViewCreateInfo& info)
{
	// TODO: implement
	return obj<TextureView>();
}

TextureView::~TextureView()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.destroy(impl.imageView);

	destroyObjectImpl(this);
}

obj<Texture> TextureView::getTexture()
{
	return getImpl(this).texture;
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