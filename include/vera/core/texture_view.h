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

class Device;
class Texture;

enum class TextureViewType VERA_ENUM
{
	Unknown,
	View1D,
	View2D,
	View3D,
	ViewCube,
	View1DArray,
	View2DArray,
	ViewCubeArray
};

enum class ComponentSwizzle VERA_ENUM
{
	Identity,
	One,
	Zero,
	R,
	G,
	B,
	A
};

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
	TextureViewType    type           = TextureViewType::Unknown;
	Format             format         = Format::Unknown;
	ComponentMapping   mapping        = VERA_COMPONENT_MAPPING(R, G, B, A);
	TextureAspectFlags aspectFlags    = TextureAspectFlagBits::Color;
	uint32_t           baseMipLevel   = 0;
	uint32_t           levelCount     = 1;
	uint32_t           baseArrayLayer = 0;
	uint32_t           layerCount     = 1;
};

class TextureView : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(TextureView)
public:
	static obj<TextureView> create(obj<Texture> texture, const TextureViewCreateInfo& info);
	~TextureView();

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
