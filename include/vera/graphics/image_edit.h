#pragma once

#include "image.h"
#include "../math/vector_types.h"
#include "../util/flag.h"
#include "../util/direction.h"

VERA_NAMESPACE_BEGIN

class ImageSampler;

enum class ImageFlipFlagBits VERA_FLAG_BITS
{
	Horizontal,
	Vertical
} VERA_ENUM_FLAGS(ImageFlipFlagBits, ImageFlipFlags)

struct ImageBlitInfo
{
	uint32_t dstWidth;
	uint32_t dstHeight;
	float2   uv0;
	float2   uv1;
	float2   uv2;
	float2   uv3;
};

struct ImageEdit
{
	static Image flip(const Image& image, bool horizontal, bool vertical);
	static Image flip(const Image& image, ImageFlipFlags flags);
	static Image rotate(const Image& image, Rotation rot);
	static Image rotateCW(const Image& image);
	static Image rotateCCW(const Image& image);
	static Image createMask(const Image& image, uint32_t at_x = 0, uint32_t at_y = 0, float alpha = 0.f, float similarity = 0.99f);
	static Image blit(const Image& image, const ImageSampler& sampler, const ImageBlitInfo& info);

	static void flip(Image& result, const Image& image, bool horizontal, bool vertical);
	static void flip(Image& result, const Image& image, ImageFlipFlags flags);
	static void rotate(Image& result, const Image& image, Rotation rot);
	static void rotateCW(Image& result, const Image& image);
	static void rotateCCW(Image& result, const Image& image);
};

VERA_NAMESPACE_END