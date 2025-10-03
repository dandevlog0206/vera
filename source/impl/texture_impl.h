#pragma once

#include "object_impl.h"

#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

struct TextureImpl
{
	obj<Device>          device;
	obj<DeviceMemory>    deviceMemory;
	obj<TextureView>     textureView;
	ref<FrameBuffer>     frameBuffer;

	vk::Image            image;

	Format               textureFormat;
	TextureUsageFlags    textureUsage;
	TextureAspectFlags   textureAspect;
	TextureLayout        textureLayout;
	uint32_t             width;
	uint32_t             height;
	uint32_t             depth;
	size_t               size;
};

struct TextureViewImpl
{
	using object_type = class TextureView;

	obj<Device>        device;
	obj<Texture>       texture;

	vk::ImageView      imageView;

	uint32_t           width;
	uint32_t           height;
	uint32_t           depth;
	TextureViewType    type;
	Format             format;
	ComponentMapping   mapping;
	TextureAspectFlags aspectFlags;
	uint32_t           baseMipLevel;
	uint32_t           levelCount;
	uint32_t           baseArrayLayer;
	uint32_t           layerCount;
};

static vk::ImageLayout to_vk_image_layout(TextureLayout layout)
{
	switch (layout) {
	case TextureLayout::Undefined:                             return vk::ImageLayout::eUndefined;
	case TextureLayout::General:                               return vk::ImageLayout::eGeneral;
	case TextureLayout::ColorAttachmentOptimal:                return vk::ImageLayout::eColorAttachmentOptimal;
	case TextureLayout::DepthStencilAttachmentOptimal:         return vk::ImageLayout::eDepthStencilAttachmentOptimal;
	case TextureLayout::DepthStencilReadOnlyOptimal:           return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
	case TextureLayout::ShaderReadOnlyOptimal:                 return vk::ImageLayout::eShaderReadOnlyOptimal;
	case TextureLayout::TransferSrcOptimal:                    return vk::ImageLayout::eTransferSrcOptimal;
	case TextureLayout::TransferDstOptimal:                    return vk::ImageLayout::eTransferDstOptimal;
	case TextureLayout::Preinitialized:                        return vk::ImageLayout::ePreinitialized;
	case TextureLayout::DepthReadOnlyStencilAttachmentOptimal: return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
	case TextureLayout::DepthAttachmentStencilReadOnlyOptimal: return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
	case TextureLayout::DepthAttachmentOptimal:                return vk::ImageLayout::eDepthAttachmentOptimal;
	case TextureLayout::DepthReadOnlyOptimal:                  return vk::ImageLayout::eDepthReadOnlyOptimal;
	case TextureLayout::StencilAttachmentOptimal:              return vk::ImageLayout::eStencilAttachmentOptimal;
	case TextureLayout::StencilReadOnlyOptimal:                return vk::ImageLayout::eStencilReadOnlyOptimal;
	case TextureLayout::ReadOnlyOptimal:                       return vk::ImageLayout::eReadOnlyOptimal;
	case TextureLayout::AttachmentOptimal:                     return vk::ImageLayout::eAttachmentOptimal;
	case TextureLayout::RenderingLocalRead:                    return vk::ImageLayout::eRenderingLocalRead;
	case TextureLayout::PresentSrc:                            return vk::ImageLayout::ePresentSrcKHR;
	}

	VERA_ASSERT_MSG(false, "invalid image layout");
	return {};
}

static vk::ImageAspectFlags to_vk_image_aspect_flags(TextureAspectFlags flags)
{
	vk::ImageAspectFlags result;

	if (flags.has(TextureAspectFlagBits::Color))
		result |= vk::ImageAspectFlagBits::eColor;
	if (flags.has(TextureAspectFlagBits::Depth))
		result |= vk::ImageAspectFlagBits::eDepth;
	if (flags.has(TextureAspectFlagBits::Stencil))
		result |= vk::ImageAspectFlagBits::eStencil;
	if (flags.has(TextureAspectFlagBits::Metadata))
		result |= vk::ImageAspectFlagBits::eMetadata;

	return result;
}

static vk::ImageUsageFlags to_vk_image_usage_flags(TextureUsageFlags flags)
{
	vk::ImageUsageFlags result;

	if (flags.has(TextureUsageFlagBits::TransferSrc))
		result |= vk::ImageUsageFlagBits::eTransferSrc;
	if (flags.has(TextureUsageFlagBits::TransferDst))
		result |= vk::ImageUsageFlagBits::eTransferDst;
	if (flags.has(TextureUsageFlagBits::Sampled))
		result |= vk::ImageUsageFlagBits::eSampled;
	if (flags.has(TextureUsageFlagBits::Storage))
		result |= vk::ImageUsageFlagBits::eStorage;
	if (flags.has(TextureUsageFlagBits::ColorAttachment))
		result |= vk::ImageUsageFlagBits::eColorAttachment;
	if (flags.has(TextureUsageFlagBits::DepthStencilAttachment))
		result |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
	if (flags.has(TextureUsageFlagBits::TransientAttachment))
		result |= vk::ImageUsageFlagBits::eTransientAttachment;
	if (flags.has(TextureUsageFlagBits::InputAttachment))
		result |= vk::ImageUsageFlagBits::eInputAttachment;
	if (flags.has(TextureUsageFlagBits::HostTransfer))
		result |= vk::ImageUsageFlagBits::eHostTransfer;
	VERA_ASSERT_MSG(!flags.has(TextureUsageFlagBits::FrameBuffer), "invalid image usage flag bit");

	return result;
}

static vk::ImageViewType to_vk_image_view_type(TextureViewType type)
{
	switch (type) {
	case TextureViewType::View1D:        return vk::ImageViewType::e1D;
	case TextureViewType::View2D:        return vk::ImageViewType::e2D;
	case TextureViewType::View3D:        return vk::ImageViewType::e3D;
	case TextureViewType::ViewCube:      return vk::ImageViewType::eCube;
	case TextureViewType::View1DArray:   return vk::ImageViewType::e1DArray;
	case TextureViewType::View2DArray:   return vk::ImageViewType::e2DArray;
	case TextureViewType::ViewCubeArray: return vk::ImageViewType::eCubeArray;
	}

	VERA_ASSERT_MSG(false, "invalid image view type");
	return {};
}

static vk::ComponentSwizzle to_vk_component_swizzle(ComponentSwizzle swizzle)
{
	switch (swizzle) {
	case ComponentSwizzle::Identity: return vk::ComponentSwizzle::eIdentity;
	case ComponentSwizzle::One:      return vk::ComponentSwizzle::eOne;
	case ComponentSwizzle::Zero:     return vk::ComponentSwizzle::eZero;
	case ComponentSwizzle::R:        return vk::ComponentSwizzle::eR;
	case ComponentSwizzle::G:        return vk::ComponentSwizzle::eG;
	case ComponentSwizzle::B:        return vk::ComponentSwizzle::eB;
	case ComponentSwizzle::A:        return vk::ComponentSwizzle::eA;
	}

	VERA_ASSERT_MSG(false, "invalid component swizzle");
	return {};
}

VERA_NAMESPACE_END