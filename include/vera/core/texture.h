#pragma once

#include "device_memory.h"
#include "../util/extent.h"

VERA_NAMESPACE_BEGIN

class TextureView;
class Image;

struct TextureCreateInfo
{
	TextureType       type        = TextureType::Texture2D;
	Format            format      = {};
	TextureUsageFlags usage       = {};
	uint32_t          width       = {};
	uint32_t          height      = {};
	uint32_t          depth       = 1;
	uint32_t          arraySize   = 1;
	uint32_t          mipLevels   = 1;
	uint32_t          sampleCount = 1;
	// ResourceBindFlags bindFlags;
};

class Texture : public CoreObject
{
	VERA_CORE_OBJECT_INIT(Texture)
public:
	static obj<Texture> createColorAtt(obj<Device> device, uint32_t width, uint32_t height, DepthFormat format);
	static obj<Texture> createDepth(obj<Device> device, uint32_t width, uint32_t height, DepthFormat format);
	static obj<Texture> createStencil(obj<Device> device, uint32_t width, uint32_t height, StencilFormat format);
	static obj<Texture> create(obj<Device> device, const Image& image);
	static obj<Texture> create(obj<Device> device, const TextureCreateInfo& info);
	~Texture() VERA_NOEXCEPT override;

	void upload(const Image& image);

	obj<Device> getDevice();
	obj<DeviceMemory> getDeviceMemory();

	ref<TextureView> getTextureView();

	Format getFormat() const;
	TextureUsageFlags getUsageFlags() const;

	uint32_t width() const;
	uint32_t height() const;
	uint32_t depth() const;
	extent3d extent() const;
};

VERA_NAMESPACE_END