#include "../../include/vera/graphics/image_sampler.h"

#include "pixel.h"
#include "../../include/vera/graphics/image.h"
#include <algorithm>

VERA_NAMESPACE_BEGIN

static float modf(float x, float y)
{
	float mod = fmodf(x, y);
	return 0 < x ? mod : y + mod;
}

static float repeatf(float x, float size)
{
	return modf(x + 0.5f, size) - 0.5f;
}

static float mirrored_repeatf(float x, float size)
{
	return -fabsf(modf(x + 0.5f, 2.f * size) - size) + size - 0.5f;
}

static float clamp_edge(float x, float size)
{
	if (x < 0.5f) return -0.5f;
	if (size - 0.5f < x) return size - 0.5f;
	return x;
}

static float clamp_border(float x, float size)
{
	if (x < -0.5f || size - 0.5f < x) return NAN;
	return x;
}

static float mirror_clamp_edge(float x, float size)
{
	return fminf(fabsf(x + 0.5f) - 0.5f, size - 0.5f);
}

template <ImageSamplerAddressMode ModeU, ImageSamplerAddressMode ModeV>
static float4 sample_nearest(const Image& image, float u, float v, const float4& border_color)
{
	const void* ptr    = image.data();
	float       width  = static_cast<float>(image.width());
	float       height = static_cast<float>(image.height());
	Format      format = image.format();

	switch (ModeU) {
	case ImageSamplerAddressMode::Repeat:
		u  = repeatf(u, width);
		break;
	case ImageSamplerAddressMode::MirroredRepeat:
		u = mirrored_repeatf(u, width);
		break;
	case ImageSamplerAddressMode::ClampToEdge:
		u = clamp_edge(u, width);
		break;
	case ImageSamplerAddressMode::ClampToBorder:
		if (u < -0.5f || width - 0.5f <= u)
			return border_color;
		break;
	case ImageSamplerAddressMode::MirrorClampToEdge:
		u = mirror_clamp_edge(u, width);
		break;
	}

	switch (ModeV) {
	case ImageSamplerAddressMode::Repeat:
		v = repeatf(v, height);
		break;
	case ImageSamplerAddressMode::MirroredRepeat:
		v = mirrored_repeatf(v, height);
		break;
	case ImageSamplerAddressMode::ClampToEdge:
		v = clamp_edge(v, height);
		break;
	case ImageSamplerAddressMode::ClampToBorder:
		if (v < -0.5f || height - 0.5f <= v)
			return border_color;
		break;
	case ImageSamplerAddressMode::MirrorClampToEdge:
		v = mirror_clamp_edge(v, height);
		break;
	}

	uint32_t rnd_u = u <= 0.5f ? 0 : static_cast<uint32_t>(roundf(u));
	uint32_t rnd_v = v <= 0.5f ? 0 : static_cast<uint32_t>(roundf(v));
	uint32_t w     = static_cast<uint32_t>(width);
	uint32_t h     = static_cast<uint32_t>(height);
	uint32_t x     = width - 1 <= u ? width - 1 : rnd_u;
	uint32_t y     = height - 1 <= v ? height - 1 : rnd_v;

	return fetch_components(ptr, w, x, y, format);
}

template <ImageSamplerAddressMode ModeU, ImageSamplerAddressMode ModeV>
static float4 sample_linear(const Image& image, float u, float v, const float4& border_color)
{
	const void* ptr    = image.data();
	float       width  = image.width();
	float       height = image.height();
	Format      format = image.format();

	switch (ModeU) {
	case ImageSamplerAddressMode::Repeat:
		u  = repeatf(u, width);
		break;
	case ImageSamplerAddressMode::MirroredRepeat:
		u = mirrored_repeatf(u, width);
		break;
	case ImageSamplerAddressMode::ClampToEdge:
		u = clamp_edge(u, width);
		break;
	case ImageSamplerAddressMode::ClampToBorder:
		if (u < -0.5f || width - 0.5f <= u)
			return border_color;
		break;
	case ImageSamplerAddressMode::MirrorClampToEdge:
		u = mirror_clamp_edge(u, width);
		break;
	}

	switch (ModeV) {
	case ImageSamplerAddressMode::Repeat:
		v = repeatf(v, height);
		break;
	case ImageSamplerAddressMode::MirroredRepeat:
		v = mirrored_repeatf(v, height);
		break;
	case ImageSamplerAddressMode::ClampToEdge:
		v = clamp_edge(v, height);
		break;
	case ImageSamplerAddressMode::ClampToBorder:
		if (v < -0.5f || height - 0.5f <= v)
			return border_color;
		break;
	case ImageSamplerAddressMode::MirrorClampToEdge:
		v = mirror_clamp_edge(v, height);
		break;
	}

	float    tx0   = fmodf(u + 1.f, 1.f);
	float    ty0   = fmodf(v + 1.f, 1.f);
	float    tx1   = 1.f - tx0;
	float    ty1   = 1.f - ty0;
	uint32_t flr_u = static_cast<uint32_t>(floorf(u));
	uint32_t flr_v = static_cast<uint32_t>(floorf(v));
	uint32_t x0    = static_cast<uint32_t>(u < 0.f ? width - 1 : flr_u);
	uint32_t x1    = static_cast<uint32_t>(width - 1.f < u ? 0 : flr_u + 1.f);
	uint32_t y0    = static_cast<uint32_t>(v < 0.f ? height- 1 : flr_v);
	uint32_t y1    = static_cast<uint32_t>(height - 1.f < v ? 0 : flr_v + 1.f);

	float4 color0 = fetch_components(ptr, static_cast<uint32_t>(width), x0, y0, format);
	float4 color1 = fetch_components(ptr, static_cast<uint32_t>(width), x1, y0, format);
	float4 color2 = fetch_components(ptr, static_cast<uint32_t>(width), x1, y1, format);
	float4 color3 = fetch_components(ptr, static_cast<uint32_t>(width), x0, y1, format);

	return
		tx0 * ty0 * color0 +
		tx1 * ty0 * color1 +
		tx1 * ty1 * color2 +
		tx0 * ty1 * color3;
}

ImageSampler::ImageSampler() :
	ImageSampler(ImageSamplerCreateInfo{}) { }

ImageSampler::ImageSampler(const ImageSamplerCreateInfo& info) :
	m_sample_fptr(nullptr),
	m_border_color(info.borderColor),
	m_unnormalized(info.unnormalizedCoordinates)
{
////////// define some macros /////////////////////////////////////////////////////////////////////

#define SAMPLE_FUNCTION_ADDRESS(function, mode_u, mode_v) \
	&function<ImageSamplerAddressMode::mode_u, ImageSamplerAddressMode::mode_v>

///////////////////////////////////////////////////////////////////////////////////////////////////

	uint32_t mode_set = static_cast<uint32_t>(info.addressModeU) + 5 * static_cast<uint32_t>(info.addressModeV);

	switch (info.filter) {
	case ImageSamplerFilter::Nearest:
		switch (mode_set) {
		case 0:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, Repeat, Repeat); break;
		case 1:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirroredRepeat, Repeat); break;
		case 2:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToEdge, Repeat); break;
		case 3:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToBorder, Repeat); break;
		case 4:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirrorClampToEdge, Repeat); break;
		case 5:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, Repeat, MirroredRepeat); break;
		case 6:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirroredRepeat, MirroredRepeat); break;
		case 7:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToEdge, MirroredRepeat); break;
		case 8:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToBorder, MirroredRepeat); break;
		case 9:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirrorClampToEdge, MirroredRepeat); break;
		case 10: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, Repeat, ClampToEdge); break;
		case 11: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirroredRepeat, ClampToEdge); break;
		case 12: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToEdge, ClampToEdge); break;
		case 13: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToBorder, ClampToEdge); break;
		case 14: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirrorClampToEdge, ClampToEdge); break;
		case 15: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, Repeat, ClampToBorder); break;
		case 16: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirroredRepeat, ClampToBorder); break;
		case 17: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToEdge, ClampToBorder); break;
		case 18: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToBorder, ClampToBorder); break;
		case 19: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirrorClampToEdge, ClampToBorder); break;
		case 20: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, Repeat, MirrorClampToEdge); break;
		case 21: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirroredRepeat, MirrorClampToEdge); break;
		case 22: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToEdge, MirrorClampToEdge); break;
		case 23: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, ClampToBorder, MirrorClampToEdge); break;
		case 24: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_nearest, MirrorClampToEdge, MirrorClampToEdge); break;
		default: VERA_ASSERT_MSG(false, "unsupported address mode");
		}
	break;
	case ImageSamplerFilter::Linear:
		switch (mode_set) {
		case 0:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, Repeat, Repeat); break;
		case 1:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirroredRepeat, Repeat); break;
		case 2:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToEdge, Repeat); break;
		case 3:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToBorder, Repeat); break;
		case 4:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirrorClampToEdge, Repeat); break;
		case 5:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, Repeat, MirroredRepeat); break;
		case 6:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirroredRepeat, MirroredRepeat); break;
		case 7:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToEdge, MirroredRepeat); break;
		case 8:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToBorder, MirroredRepeat); break;
		case 9:  m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirrorClampToEdge, MirroredRepeat); break;
		case 10: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, Repeat, ClampToEdge); break;
		case 11: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirroredRepeat, ClampToEdge); break;
		case 12: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToEdge, ClampToEdge); break;
		case 13: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToBorder, ClampToEdge); break;
		case 14: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirrorClampToEdge, ClampToEdge); break;
		case 15: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, Repeat, ClampToBorder); break;
		case 16: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirroredRepeat, ClampToBorder); break;
		case 17: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToEdge, ClampToBorder); break;
		case 18: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToBorder, ClampToBorder); break;
		case 19: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirrorClampToEdge, ClampToBorder); break;
		case 20: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, Repeat, MirrorClampToEdge); break;
		case 21: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirroredRepeat, MirrorClampToEdge); break;
		case 22: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToEdge, MirrorClampToEdge); break;
		case 23: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, ClampToBorder, MirrorClampToEdge); break;
		case 24: m_sample_fptr = SAMPLE_FUNCTION_ADDRESS(sample_linear, MirrorClampToEdge, MirrorClampToEdge); break;
		default: VERA_ASSERT_MSG(false, "unsupported address mode");
		}
	break;
	}

#undef SAMPLE_FUNCTION_ADDRESS
}

float4 ImageSampler::sample(const Image& image, const float2& uv) const
{
	return sample(image, uv.x, uv.y);
}

float4 ImageSampler::sample(const Image& image, float u, float v) const
{
	if (!m_unnormalized) {
		u *= image.width();
		v *= image.height();
	}

	return m_sample_fptr(image, u, v, m_border_color);
}

VERA_NAMESPACE_END