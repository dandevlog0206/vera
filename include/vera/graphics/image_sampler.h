#pragma once

#include "color.h"

VERA_NAMESPACE_BEGIN

class Image;

enum class ImageSamplerFilter
{
	Nearest,
	Linear,
	Area
};

enum class ImageSamplerAddressMode
{
	Repeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder,
	MirrorClampToEdge
};

struct ImageSamplerCreateInfo
{
	ImageSamplerFilter      filter                  = ImageSamplerFilter::Nearest;
	ImageSamplerAddressMode addressModeU            = ImageSamplerAddressMode::Repeat;
	ImageSamplerAddressMode addressModeV            = ImageSamplerAddressMode::Repeat;
	float4                  borderColor             = { 0.f, 0.f, 0.f, 1.f };
	bool                    unnormalizedCoordinates = false;
};

class ImageSampler
{
public:
	ImageSampler();
	ImageSampler(const ImageSamplerCreateInfo& info);

	float4 sample(const Image& image, const float2& uv) const;
	float4 sample(const Image& image, float u, float v) const;

private:
	typedef float(*AddressModeFPtr)(float, float);
	typedef float4(*SampleFPtr)(const Image&, float, float);

	AddressModeFPtr m_adress_u_fptr;
	AddressModeFPtr m_adress_v_fptr;
	SampleFPtr      m_sample_fptr;
	float4          m_border_color;
	bool            m_unnormalized;
};

VERA_NAMESPACE_END