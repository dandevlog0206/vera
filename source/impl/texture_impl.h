#pragma once

#include "object_impl.h"

#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

class TextureImpl
{
public:
	obj<Device>          device        = {};
	obj<DeviceMemory>    deviceMemory  = {};
	obj<TextureView>     textureView   = {};
	ref<FrameBuffer>     frameBuffer   = {};

	vk::Image            image         = {};

	Format               textureFormat = {};
	TextureUsageFlags    textureUsage  = {};
	TextureAspectFlags   textureAspect = {};
	TextureLayout        textureLayout = {};
	uint32_t             width         = {};
	uint32_t             height        = {};
	uint32_t             depth         = {};
	size_t               size          = {};
};

class TextureViewImpl
{
public:
	obj<Device>        device         = {};
	obj<Texture>       texture        = {};

	vk::ImageView      imageView      = {};

	uint32_t           width          = {};
	uint32_t           height         = {};
	uint32_t           depth          = {};
	TextureViewType    type           = {};
	Format             format         = {};
	ComponentMapping   mapping        = {};
	TextureAspectFlags aspectFlags    = {};
	uint32_t           baseMipLevel   = {};
	uint32_t           levelCount     = {};
	uint32_t           baseArrayLayer = {};
	uint32_t           layerCount     = {};
};

VERA_NAMESPACE_END