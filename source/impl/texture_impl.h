#pragma once

#include "object_impl.h"

#include "../../include/vera/core/texture.h"

VERA_NAMESPACE_BEGIN

struct TextureImpl
{
	obj<Device>          device;
	obj<DeviceMemory>    deviceMemory;
	obj<TextureView>     textureView;

	vk::Image            image;
	vk::ImageAspectFlags imageAspect;
	vk::ImageLayout      imageLayout;

	ImageUsageFlags      imageUsage;
	Format               imageFormat;
	uint32_t             width;
	uint32_t             height;
	uint32_t             depth;
	size_t               size;
};

struct TextureViewImpl
{
	using object_type = class TextureView;

	obj<Device>   device;
	obj<Texture>  texture;

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

static vk::ImageUsageFlags to_vk_image_usage_flags(ImageUsageFlags flags)
{
	vk::ImageUsageFlags result;

	if (flags.has(ImageUsageFlagBits::TransferSrc))
		result |= vk::ImageUsageFlagBits::eTransferSrc;
	if (flags.has(ImageUsageFlagBits::TransferDst))
		result |= vk::ImageUsageFlagBits::eTransferDst;
	if (flags.has(ImageUsageFlagBits::Sampled))
		result |= vk::ImageUsageFlagBits::eSampled;
	if (flags.has(ImageUsageFlagBits::Storage))
		result |= vk::ImageUsageFlagBits::eStorage;
	if (flags.has(ImageUsageFlagBits::ColorAttachment))
		result |= vk::ImageUsageFlagBits::eColorAttachment;
	if (flags.has(ImageUsageFlagBits::DepthStencilAttachment))
		result |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
	if (flags.has(ImageUsageFlagBits::TransientAttachment))
		result |= vk::ImageUsageFlagBits::eTransientAttachment;
	if (flags.has(ImageUsageFlagBits::InputAttachment))
		result |= vk::ImageUsageFlagBits::eInputAttachment;
	if (flags.has(ImageUsageFlagBits::HostTransfer))
		result |= vk::ImageUsageFlagBits::eHostTransfer;
	if (flags.has(ImageUsageFlagBits::SwapchainImage))
		VERA_ASSERT_MSG(false, "invalid image usage flag bit");

	return result;
}

VERA_NAMESPACE_END