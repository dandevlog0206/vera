#pragma once

#include "texture.h"

VERA_NAMESPACE_BEGIN

struct TextureViewCreateInfo
{
	TextureType       type;
	Format            format;
	uint32_t          width;
	uint32_t          height;
	uint32_t          depth;
	uint32_t          arraySize;
	uint32_t          mipLevels;
	uint32_t          sampleCount;
};

class TextureView : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(TextureView)
public:
	static obj<TextureView> create(obj<Texture> texture, const TextureViewCreateInfo& info);
	~TextureView();

	obj<Device> getDevice();
	obj<Texture> getTexture();

	uint32_t width() const;
	uint32_t height() const;
	uint32_t depth() const;
	extent3d extent() const;
};

VERA_NAMESPACE_END
