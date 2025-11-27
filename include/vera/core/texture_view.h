#pragma once

#include "texture.h"
#include "../util/extent.h"

#define VERA_COMPONENT_MAPPING(r, g, b, a) \
	ComponentMapping{                      \
		ComponentSwizzle::r,               \
		ComponentSwizzle::g,               \
		ComponentSwizzle::b,               \
		ComponentSwizzle::a }

VERA_NAMESPACE_BEGIN

class Texture;

struct ComponentMapping
{
	ComponentSwizzle r = ComponentSwizzle::Identity;
	ComponentSwizzle g = ComponentSwizzle::Identity;
	ComponentSwizzle b = ComponentSwizzle::Identity;
	ComponentSwizzle a = ComponentSwizzle::Identity;
};

struct TextureSubresourceRange
{
	TextureAspectFlags aspectFlags;
	uint32_t           baseMipLevel;
	uint32_t           levelCount;
	uint32_t           baseArrayLayer;
	uint32_t           layerCount;
};

struct TextureViewCreateInfo
{
	TextureViewType    type           = TextureViewType::View2D;
	Format             format         = Format::Unknown;
	ComponentMapping   mapping        = VERA_COMPONENT_MAPPING(R, G, B, A);
	TextureAspectFlags aspectFlags    = TextureAspectFlagBits::Color;
	uint32_t           baseMipLevel   = 0;
	uint32_t           levelCount     = 1;
	uint32_t           baseArrayLayer = 0;
	uint32_t           layerCount     = 1;
};

class TextureView : public CoreObject
{
	VERA_CORE_OBJECT_INIT(TextureView)
public:
	static obj<TextureView> create(obj<Texture> texture, const TextureViewCreateInfo& info = {});
	~TextureView() VERA_NOEXCEPT override;

	obj<Device> getDevice();
	obj<Texture> getTexture();

	TextureViewType getType() const;
	Format getFormat() const;
	ComponentMapping getComponentMapping() const;
	TextureSubresourceRange getTextureSubresourceRange() const;

	uint32_t width() const;
	uint32_t height() const;
	uint32_t depth() const;
	extent3d extent() const;
};

VERA_NAMESPACE_END
