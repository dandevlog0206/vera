#pragma once

#include "core_object.h"
#include "../graphics/color.h"

VERA_NAMESPACE_BEGIN

class Device;

enum class SamplerFilter
{
	Nearest,
	Linear
};

enum class SamplerMipmapMode
{
	Nearest,
	Linear
};

enum class SamplerAddressMode
{
	Repeat,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder,
	MirrorClampToEdge
};

enum class SamplerCompareOp
{
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always
};

struct SamplerCreateInfo
{
	SamplerFilter      magFilter               = SamplerFilter::Linear;
	SamplerFilter      minFilter               = SamplerFilter::Linear;
	SamplerMipmapMode  mipmapMode              = SamplerMipmapMode::Linear;
	SamplerAddressMode addressModeU            = SamplerAddressMode::Repeat;
	SamplerAddressMode addressModeV            = SamplerAddressMode::Repeat;
	SamplerAddressMode addressModeW            = SamplerAddressMode::Repeat;
	float              mipLodBias              = 0.f;
	bool               anisotropyEnable        = false;
	float              maxAnisotropy           = 0.f;
	bool               compareEnable           = false;
	SamplerCompareOp   compareOp               = SamplerCompareOp::Never;
	float              minLod                  = 0.f;
	float              maxLod                  = 0.f;
	Color              borderColor             = Colors::Black;
	bool               unnormalizedCoordinates = false;
};

class Sampler : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Sampler)
public:
	static obj<Sampler> create(obj<Device> device, const SamplerCreateInfo& info = {});
	~Sampler();

	obj<Device> getDevice();

	const SamplerCreateInfo& getInfo() const;

	size_t hash() const;
};

VERA_NAMESPACE_END