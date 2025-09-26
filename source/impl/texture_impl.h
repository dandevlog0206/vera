#pragma once

#include "object_impl.h"

#include "../../include/vera/core/texture.h"

VERA_NAMESPACE_BEGIN

struct TextureImpl
{
	ref<Device>          device;
	ref<DeviceMemory>    deviceMemory;
	ref<TextureView>     textureView;

	vk::Image            image;
	vk::ImageUsageFlags  imageUsage;
	vk::ImageAspectFlags imageAspect;
	vk::ImageLayout      imageLayout;

	Format               imageFormat;
	uint32_t             width;
	uint32_t             height;
	uint32_t             depth;
	bool                 isSwapchainImage;
};

struct TextureViewImpl
{
	using object_type = class TextureView;

	ref<Device>   device;
	ref<Texture>  texture;

	vk::ImageView imageView;

	uint32_t      offsetX;
	uint32_t      offsetY;
	uint32_t      width;
	uint32_t      height;
	uint32_t      depth;
};

static vk::ImageLayout to_vk_image_layout(ImageLayout layout)
{
	switch (layout) {
	case ImageLayout::Undefined:                             return vk::ImageLayout::eUndefined;
	case ImageLayout::General:                               return vk::ImageLayout::eGeneral;
	case ImageLayout::ColorAttachmentOptimal:                return vk::ImageLayout::eColorAttachmentOptimal;
	case ImageLayout::DepthStencilAttachmentOptimal:         return vk::ImageLayout::eDepthStencilAttachmentOptimal;
	case ImageLayout::DepthStencilReadOnlyOptimal:           return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
	case ImageLayout::ShaderReadOnlyOptimal:                 return vk::ImageLayout::eShaderReadOnlyOptimal;
	case ImageLayout::TransferSrcOptimal:                    return vk::ImageLayout::eTransferSrcOptimal;
	case ImageLayout::TransferDstOptimal:                    return vk::ImageLayout::eTransferDstOptimal;
	case ImageLayout::Preinitialized:                        return vk::ImageLayout::ePreinitialized;
	case ImageLayout::DepthReadOnlyStencilAttachmentOptimal: return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
	case ImageLayout::DepthAttachmentStencilReadOnlyOptimal: return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
	case ImageLayout::DepthAttachmentOptimal:                return vk::ImageLayout::eDepthAttachmentOptimal;
	case ImageLayout::DepthReadOnlyOptimal:                  return vk::ImageLayout::eDepthReadOnlyOptimal;
	case ImageLayout::StencilAttachmentOptimal:              return vk::ImageLayout::eStencilAttachmentOptimal;
	case ImageLayout::StencilReadOnlyOptimal:                return vk::ImageLayout::eStencilReadOnlyOptimal;
	case ImageLayout::ReadOnlyOptimal:                       return vk::ImageLayout::eReadOnlyOptimal;
	case ImageLayout::AttachmentOptimal:                     return vk::ImageLayout::eAttachmentOptimal;
	case ImageLayout::RenderingLocalRead:                    return vk::ImageLayout::eRenderingLocalRead;
	case ImageLayout::PresentSrc:                            return vk::ImageLayout::ePresentSrcKHR;
	}

	VERA_ASSERT_MSG(false, "invalid image layout");
	return {};
}

VERA_NAMESPACE_END