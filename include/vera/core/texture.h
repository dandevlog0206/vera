#pragma once

#include "core_object.h"
#include "../graphics/format.h"
#include "../util/extent.h"

VERA_NAMESPACE_BEGIN

class Device;
class DeviceMemory;
class TextureView;
class Image;

enum class TextureType VERA_ENUM
{
	Texture1D,
	Texture2D,
	Texture3D,
	TextureCube,
	Texture2DMultisample,
};

enum class ImageLayout VERA_ENUM
{
	Undefined,
	General,
	ColorAttachmentOptimal,
	DepthStencilAttachmentOptimal,
	DepthStencilReadOnlyOptimal,
	ShaderReadOnlyOptimal,
	TransferSrcOptimal,
	TransferDstOptimal,
	Preinitialized,
	DepthReadOnlyStencilAttachmentOptimal,
	DepthAttachmentStencilReadOnlyOptimal,
	DepthAttachmentOptimal,
	DepthReadOnlyOptimal,
	StencilAttachmentOptimal,
	StencilReadOnlyOptimal,
	ReadOnlyOptimal,
	AttachmentOptimal,
	RenderingLocalRead,
	PresentSrc
};

struct TextureCreateInfo
{
	TextureType       type        = TextureType::Texture2D;
	Format            format      = {};
	uint32_t          width       = {};
	uint32_t          height      = {};
	uint32_t          depth       = 1;
	uint32_t          arraySize   = 1;
	uint32_t          mipLevels   = 1;
	uint32_t          sampleCount = 1;
	// ResourceBindFlags bindFlags;
};

class Texture : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Texture)
public:
	static obj<Texture> create(obj<Device> device, const TextureCreateInfo& info);
	~Texture();

	void upload(const Image& image);

	obj<Device> getDevice();
	obj<DeviceMemory> getDeviceMemory();

	ref<TextureView> getTextureView();

	uint32_t width() const;
	uint32_t height() const;
	uint32_t depth() const;
	extent3d extent() const;
};

VERA_NAMESPACE_END